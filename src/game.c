#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <stdbool.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

int nScreenWidth = 160;
int nScreenHeight = 40;
int nMapWidth = 27;
int nMapHeight = 27;

float fPlayerX = 25.99f;
float fPlayerY = 10.70f;
float fEnemyX = 3.48f;
float fEnemyY = 16.92f;
float fPlayerA = 4.0f;
float fFOV = 3.14159f / 4.0f;
float fDepth = 27.0f;
float fSpeed = 5.0f;

bool bEscKeyPressed = false;
bool bEscKeyPressedPrev = false;

char map[] =
    "####################%######"
    "#.....#....##.....#....#..#"
    "#.....#....##.....#....#..#"
    "#..#..#...............#...#"
    "#..#..#...................#"
    "#..#..#...........#....#..#"
    "#..#.......##.....###..#..#"
    "#..........####.........###"
    "#######..#######..####....#"
    "#######..#######..#########"
    "#.....#....##.....#....#..#"
    "#.....#....##.....#....#..#"
    "#..#..#..###..#..#..#.....#"
    "#..#..#........#..#....#..#"
    "#..#..#....##..#..#....#..#"
    "#..#..###..##..#..###..#..#"
    "####.......#####.......####"
    "#######..#######..####....#"
    "#######..################.#"
    "#.....#....##.....#....##.#"
    "#.....#....##.....#.......#"
    "#..#..#..###..#.....###...#"
    "#..#..#........#..#....#..#"
    "#..#..#....##..#..#....#..#"
    "#..#..###..##..#..###..#..#"
    "####.......######.......###"
    "###########################";

    bool isVisible(float ex, float ey, float tx, float ty) {
        float dx = tx - ex;
        float dy = ty - ey;
        float distance = sqrtf(dx * dx + dy * dy);
        float stepX = dx / distance;
        float stepY = dy / distance;
        for (float i = 0.0f; i < distance; i += 0.1f) {
            int cx = (int)(ex + stepX * i);
            int cy = (int)(ey + stepY * i);
            if (map[cx * nMapWidth + cy] == '#') return false;
        }
        return true;
    }
    
    void moveEnemy(float *ex, float *ey, float tx, float ty, float dt) {
        float dx = tx - *ex;
        float dy = ty - *ey;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 0.1f) return;
        
        float step = 1.0f * dt;
        float stepX = step * (dx / dist);
        float stepY = step * (dy / dist);
        int nx = (int)(*ex + stepX);
        int ny = (int)(*ey + stepY);
    
        if (map[nx * nMapWidth + ny] != '#') {
            *ex += stepX;
            *ey += stepY;
        }
    }

    int calculateScore(int deaths, float time) {
    return (int)(10000 / (1 + deaths * 10 + time));
    }

    float fEnemyDirAngle = 0.0f;
    float fEnemyDirTimer = 0.0f;

    typedef struct {
    wchar_t name[50];
    int deaths;
    float time;
    int score;
    } PlayerScore;

    #define MAX_SCORES 10
    PlayerScore scores[MAX_SCORES];
    int scoreCount = 0;

    wchar_t currentPlayerName[50] = L"";
    int currentDeaths = 0;
    float currentGameTime = 0.0f;
    bool nameEntered = false;

    typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_ENTER_NAME,
    STATE_PAUSE,
    STATE_WIN
    } GameState;

    GameState currentState = STATE_MENU;

void addScore(const wchar_t *name, int deaths, float time) {
    if (scoreCount < MAX_SCORES) {
        wcscpy(scores[scoreCount].name, name);
        scores[scoreCount].deaths = deaths;
        scores[scoreCount].time = time;
        scores[scoreCount].score = calculateScore(deaths, time);
        scoreCount++;
    } else {

        int minIndex = 0;
        for (int i = 1; i < MAX_SCORES; i++) {
            if (scores[i].score < scores[minIndex].score) {
                minIndex = i;
            }
        }
        

        if (calculateScore(deaths, time) > scores[minIndex].score) {
            wcscpy(scores[minIndex].name, name);
            scores[minIndex].deaths = deaths;
            scores[minIndex].time = time;
            scores[minIndex].score = calculateScore(deaths, time);
        }
    }
    

    for (int i = 0; i < scoreCount - 1; i++) {
        for (int j = i + 1; j < scoreCount; j++) {
            if (scores[j].score > scores[i].score) {
                PlayerScore temp = scores[i];
                scores[i] = scores[j];
                scores[j] = temp;
            }
        }
    }
}


void drawMenu(wchar_t *screen, short *colors) {
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
        screen[i] = L' ';
        colors[i] = BACKGROUND_BLUE;
    }
    const wchar_t *asciiTitle[] = {
        L":::::::.   :::         ...     :::::::. :: .::::::.   :::::::-.   ...    ::::::.    :::.  .,-:::::/ .,::::::     ...   :::.    :::.",
        L" ;;;'';;'  ;;;      .;;;;;;;.   ;;;'';;',';;;`    `    ;;,   `';, ;;     ;;;`;;;;,  `;;;,;;-'````'  ;;;;''''  .;;;;;;;.`;;;;,  `;;;",
        L" [[[__[[\\. [[[     ,[[     \\[[, [[[__[[\\. '[==/[[[[,   `[[     [[[['     [[[  [[[[[. '[[[[[   [[[[[[/[[cccc  ,[[     \\[[,[[[[[. '[[",
        L" $$\"\"\"\"Y$$ $$'     $$$,     $$$ $$\"\"\"\"Y$$   '''    $    $$,    $$$$      $$$  $$$ \"Y$c$$\"$$c.    \"$\" $$\"\"\"\"  $$$,     $$$$$$ \"Y$c$$",
        L"_88o,,od8Po88oo,.__\"888,_ _,88P_88o,,od8P  88b    dP    888_,o8P'88    .d888  888    Y88 `Y8bo,,,o88o888oo,__\"888,_ _,88P888    Y88",
        L"\"\"YUMMMP\" \"\"\"\"YUMMM  \"YMMMMMP\" \"\"YUMMMP\"    \"YMmMY\"     MMMMP\"`   \"YmmMMMM\"\"  MMM     YM   `'YMUP\"YMM\"\"\"\"YUMMM \"YMMMMMP\" MMM     YM"
    };

    int titleArtHeight = sizeof(asciiTitle) / sizeof(asciiTitle[0]);
    int titleArtStartY = 2;
    for (int i = 0; i < titleArtHeight; i++) {
        int artLen = wcslen(asciiTitle[i]);
        int startX = (nScreenWidth - artLen) / 2;
        int offset = (titleArtStartY + i) * nScreenWidth + startX;
        for (int j = 0; j < artLen; j++) {
            screen[offset + j] = asciiTitle[i][j];
            colors[offset + j] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
        }
    }

    const wchar_t *option1 = L"1. Iniciar Jogo";
    const wchar_t *option2 = L"2. Sair";
    int optionStartY = titleArtStartY + titleArtHeight + 1;
    int optionStartX = (nScreenWidth - wcslen(option1)) / 2;

    int offset1 = optionStartY * nScreenWidth + optionStartX;
    for (int j = 0; j < wcslen(option1); j++) {
        screen[offset1 + j] = option1[j];
        colors[offset1 + j] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
    }

    int offset2 = (optionStartY + 1) * nScreenWidth + optionStartX;
    for (int j = 0; j < wcslen(option2); j++) {
        screen[offset2 + j] = option2[j];
        colors[offset2 + j] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
    }

    const wchar_t *artImage[] = {
        L"                ###############################             ",
        L"            ######################################          ",
        L"          ##########################################        ",
        L"        #############################################       ",
        L"       ###############################################      ",
        L"      ############000####################000############    ",
        L"    ###############000##################000###############  ",
        L"    #########__################################__#########  ",
        L"  #############_____######################_____#############",
        L"  ##################___################___##################",
        L"  #####################_______##_______#####################",
        L"  ############################__############################",
        L"  ##########################################################",
        L"  ##########################################################",
        L"    ######################################################  ",
        L"      ##################################################    ",
        L"        ##############################################      ",
        L"          ##########################################        "
    };
    int artImageHeight = sizeof(artImage) / sizeof(artImage[0]);
    int artImageStartY = optionStartY + 4;
    for (int i = 0; i < artImageHeight; i++) {
        int lineLen = wcslen(artImage[i]);
        int startX = (nScreenWidth - lineLen) / 2;
        int offset = (artImageStartY + i) * nScreenWidth + startX;
        for (int j = 0; j < lineLen; j++) {
            screen[offset + j] = artImage[i][j];
            colors[offset + j] = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
            if (artImage[i][j] == L'#')
                colors[offset + j] = FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_BLUE;
        }
    }

    const wchar_t *fireArt[] = {
        L"            --                              MM::++mmmmMM      MMmmmm            ..mm++++....++  --  --..                        ++++MM      ++++--  ..  ....                mm                          ",
        L"            ++mm        ::::mm            mm++mm++++++::mm++MM++MMmm    ::        ..mm::::..++::--++..      --    ::      --..--  --::mm  ----......  ......                ::++::----    ::           ",
        L"            --          mm::        mmmmmm    MMmmMM--mm@@@@mmMM++MM--++MMmm----mm@@MMMM@@MMmm--++++::mm::----....mm--....@@mmMM::mm..--::++mm::::--..mm----..mmmmMM@@MMmm++          ::--              ",
        L"          ::..            ++        MMmmmm    MM@@++MMmm++MMmmmm++::--::::mm::--::MM++mm####@@MMMMmm--::::..------..::mm::MM@@mm..::....::--::..--::::..  --    ++@@mmMMmm--          --                ",
        L"        ++....            ::    ::    ::::  mmmmmm  MMMMmm++++--::----::--::--::--..++MM@@@@@@MMMM++--::..--  --mm  ----MM@@MM--::::::MM++..--....--mmmm..::  mmmmMMMM::++..--::      ::..              ",
        L"        ..              mm::++  ++    ++++@@++::++++@@@@mm--++++++::..::mm..  ::--..++MMmmMM@@++MM++--++......::--    mm##@@@@--++----MMmm..++  ..++++mm::++++--++mm@@mm----..++..    ::++++....        ",
        L"        ++--::  --....----....mmmmmm--mm++++++mmmmmm----++----@@mm....--..::::      mmMMMMMMMM++mm++--::--::mm..  ..  MMmmMMMM..--++mm::++..++--..++mmMM::--++++++++MMMM--mm--::::++++::++++::....      ",
        L"        mm++mm++MMmmMM++mmmmmmmm@@mmMM..++++mmmmMMmmMM::::mm::MMMMmm::++::++::--::::MMmmmm++mmmmmmmmMMMM++mmMM++mm++mm::++++++MM++++::mmmm++mm++mmMM@@MM##MMMMmm@@@@mm++++mm++::++--mmmm++@@mm++++      "
    };

    int fireArtHeight = sizeof(fireArt) / sizeof(fireArt[0]);
    int fireStartY = nScreenHeight - fireArtHeight - 1;
    for (int i = 0; i < fireArtHeight; i++) {
            int lineLen = wcslen(fireArt[i]);
            int startX = (nScreenWidth - lineLen) / 2;
            int offset = (fireStartY + i) * nScreenWidth + startX;
            for (int j = 0; j < lineLen; j++) {
                screen[offset + j] = fireArt[i][j];
                colors[offset + j] |= FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        }
    }
}

void drawWin(wchar_t *screen) {
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
        screen[i] = L' ';

    const wchar_t *trophy[] = {
        L"            ###########################           ",
        L"            ###########################           ",
        L"            ###########################           ",
        L"            ###########################           ",
        L"     #########################################    ",
        L"   ####     ###########################     ####  ",
        L"   ####      #########################      ####  ",
        L"     ###      #######################      ###    ",
        L"       ####     ###################     ####      ",
        L"         #################################        ",
        L"              ########################            ",
        L"                  ###############                 ",
        L"                    ###########                   ",
        L"                     #########                    ",
        L"                       #####                      ",
        L"                      #######                     ",
        L"                   #############                  ",
        L"                  ###############                 ",
        L"                 #################                ",
        L"                ###################               "
    };

    const wchar_t *title = L"PARABÉNS, CAMPEÃO!";
    const wchar_t *subtitle = L"Você conquistou o labirinto!";
    wchar_t scoreText[100];
    _snwprintf(scoreText, 100, L"%d (Mortes: %d, Tempo: %.1fs)", calculateScore(currentDeaths, currentGameTime), currentDeaths, currentGameTime);
    
    const wchar_t *option = L"Pressione [M] para voltar ao Menu";

    int startY = nScreenHeight / 2 - 15;
    int trophyX = (nScreenWidth - wcslen(trophy[0])) / 2;

    for (int i = 0; i < 20; i++) {
        _snwprintf(&screen[(startY + i) * nScreenWidth + trophyX], 
                  nScreenWidth, L"%s", trophy[i]);
    }

    int textY = startY + 20;
    int titleX = (nScreenWidth - wcslen(title)) / 2;
    int subtitleX = (nScreenWidth - wcslen(subtitle)) / 2;
    int scoreX = (nScreenWidth - wcslen(scoreText)) / 2;
    int optionX = (nScreenWidth - wcslen(option)) / 2;

    _snwprintf(&screen[textY * nScreenWidth + titleX], nScreenWidth, L"%s", title);
    _snwprintf(&screen[(textY + 2) * nScreenWidth + subtitleX], nScreenWidth, L"%s", subtitle);
    
    _snwprintf(&screen[(textY + 4) * nScreenWidth + scoreX], nScreenWidth, L"%s", scoreText);
              

    const wchar_t *rankingTitle = L"=== TOP JOGADORES ===";
    int rankingX = (nScreenWidth - wcslen(rankingTitle)) / 2;
    _snwprintf(&screen[(textY + 6) * nScreenWidth + rankingX], nScreenWidth, L"%s", rankingTitle);
    
    for (int i = 0; i < (scoreCount < 5 ? scoreCount : 5); i++) {
        wchar_t rankEntry[100];
        _snwprintf(rankEntry, 100, L"%d. %s - %d pts (Mortes: %d, Tempo: %.1fs)", i+1, scores[i].name, scores[i].score, scores[i].deaths, scores[i].time);
        int entryX = (nScreenWidth - wcslen(rankEntry)) / 2;
        _snwprintf(&screen[(textY + 8 + i) * nScreenWidth + entryX], nScreenWidth, L"%s", rankEntry);
    }
    _snwprintf(&screen[(textY + 4) * nScreenWidth + optionX], nScreenWidth, L"%s", option);
}

void drawGameOver(wchar_t *screen) {
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
        screen[i] = L' ';
    }

    const wchar_t *gameOverArt[] = {
        L"   _____          __  __ ______    ______      ________ _____  ",
        L"  / ____|   /\\   |  \\/  |  ____|  / __ \\ \\    / /  ____|  __ \\ ",
        L" | |  __   /  \\  | \\  / | |__    | |  | \\ \\  / /| |__  | |__) |",
        L" | | |_ | / /\\ \\ | |\\/| |  __|   | |  | |\\ \\/ / |  __| |  _  / ",
        L" | |__| |/ ____ \\| |  | | |____  | |__| | \\  /  | |____| | \\ \\ ",
        L"  \\_____/_/    \\_\\_|  |_|______|  \\____/   \\/   |______|_|  \\_\\"
    };

    const wchar_t *options[] = {
        L"[R] Reiniciar Jogo",
        L"[M] Voltar ao Menu"
    };

    int startY = nScreenHeight / 2 - 6;
    int artX = (nScreenWidth - wcslen(gameOverArt[0])) / 2;

    for (int i = 0; i < 6; i++) {
        _snwprintf(&screen[(startY + i) * nScreenWidth + artX], 
                  nScreenWidth, L"%s", gameOverArt[i]);
    }

    int optionY = startY + 8;
    for (int i = 0; i < 2; i++) {
        int optionX = (nScreenWidth - wcslen(options[i])) / 2;
        _snwprintf(&screen[(optionY + i*2) * nScreenWidth + optionX], 
                  nScreenWidth, L"%s", options[i]);
    }
}

int main() {
    wchar_t *screen = malloc(sizeof(wchar_t) * nScreenWidth * nScreenHeight);
    WORD *colors = malloc(sizeof(WORD) * nScreenWidth * nScreenHeight);
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    
    COORD coord = { nScreenWidth, nScreenHeight };
    SetConsoleScreenBufferSize(hConsole, coord);
    SMALL_RECT rect = { 0, 0, nScreenWidth - 1, nScreenHeight - 1 };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);

    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    bool enemyChasing = false;
    bool menuMusicPlaying = false;
    bool playMusicPlaying = false;
    bool gameoverMusicPlaying = false;
    bool victoryMusicPlaying = false;

    LARGE_INTEGER frequency, t1, t2;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&t1);

    while (1) {
        QueryPerformanceCounter(&t2);
        float fElapsedTime = (float)(t2.QuadPart - t1.QuadPart) / frequency.QuadPart;
        t1 = t2;

        bEscKeyPressedPrev = bEscKeyPressed;
        bEscKeyPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        
        switch (currentState)
        {
        case STATE_MENU:

            currentDeaths = 0;
            currentGameTime = 0.0f;
            nameEntered = false;

            victoryMusicPlaying = false;
            gameoverMusicPlaying = false;

            if (!menuMusicPlaying) {
                PlaySound(TEXT("assets/intro.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                menuMusicPlaying = true;
            }

            drawMenu(screen, colors);

            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);

            if (GetAsyncKeyState('1') & 0x8000) {
                PlaySound(NULL, 0, 0);
                menuMusicPlaying = false;
                currentState = STATE_ENTER_NAME;
                Sleep(200);
            } else if (GetAsyncKeyState('2') & 0x8000) {
                free(screen);
                free(colors);
                return 0;
            }
            break;
        
        case STATE_ENTER_NAME:
            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
                screen[i] = L' ';
                colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            }
                
            const wchar_t* prompt = L"Digite seu nome e pressione ENTER:";
            int promptX = (nScreenWidth - wcslen(prompt)) / 2;
            _snwprintf(&screen[10 * nScreenWidth + promptX], 
                    nScreenWidth, L"%s", prompt);

            int nameX = (nScreenWidth - wcslen(currentPlayerName)) / 2;
            _snwprintf(&screen[12 * nScreenWidth + nameX], 
                    nScreenWidth, L"%s", currentPlayerName);

            for (int i = 0; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i == VK_RETURN && wcslen(currentPlayerName) > 0) {
                        nameEntered = true;
                        currentState = STATE_PLAYING;
                        playMusicPlaying = false;
                        Sleep(200);
                    } 
                    else if (i == VK_BACK && wcslen(currentPlayerName) > 0) {
                        currentPlayerName[wcslen(currentPlayerName)-1] = L'\0';
                        Sleep(100);
                    }
                    else if ((i >= 'A' && i <= 'Z') || (i >= '0' && i <= '9') || i == ' ' && wcslen(currentPlayerName) < 49) {

                        wchar_t c = (wchar_t)i;
                        size_t len = wcslen(currentPlayerName);
                        currentPlayerName[len] = c;
                        currentPlayerName[len+1] = L'\0';
                        Sleep(100);
                    }
                }
            }
                
            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            break;
            

        case STATE_PLAYING:
            
            currentGameTime += fElapsedTime;    

            if (!playMusicPlaying) {
                PlaySound(TEXT("assets/play.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
                playMusicPlaying = true;
            }
            
            if (bEscKeyPressed && !bEscKeyPressedPrev) {
                currentState = STATE_PAUSE;
                Sleep(100);
                continue;
            }

            if (GetAsyncKeyState('A') & 0x8000)
                fPlayerA -= (fSpeed * 0.75f) * fElapsedTime;
            if (GetAsyncKeyState('D') & 0x8000)
                fPlayerA += (fSpeed * 0.75f) * fElapsedTime;

            if (GetAsyncKeyState('W') & 0x8000) {
                float fNewX = fPlayerX + sinf(fPlayerA) * fSpeed * fElapsedTime;
                float fNewY = fPlayerY + cosf(fPlayerA) * fSpeed * fElapsedTime;
                if (map[(int)fNewX * nMapWidth + (int)fNewY] != '#') {
                    fPlayerX = fNewX;
                    fPlayerY = fNewY;
                }
            }

            if (GetAsyncKeyState('S') & 0x8000) {
                float fNewX = fPlayerX - sinf(fPlayerA) * fSpeed * fElapsedTime;
                float fNewY = fPlayerY - cosf(fPlayerA) * fSpeed * fElapsedTime;
                if (map[(int)fNewX * nMapWidth + (int)fNewY] != '#') {
                    fPlayerX = fNewX;
                    fPlayerY = fNewY;
                }
            }

            if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '%') {
                
                addScore(currentPlayerName, currentDeaths, currentGameTime);
                currentState = STATE_WIN;
                continue;
            }

            if ((int)fPlayerX == (int)fEnemyX && (int)fPlayerY == (int)fEnemyY) {
                
                currentDeaths++;

                PlaySound(NULL, 0, 0);
                playMusicPlaying = false;
 

                fPlayerX = 25.99f;
                fPlayerY = 10.70f;
                fEnemyX = 3.48f;
                fEnemyY = 16.92f;
                fPlayerA = 4.0f;
                enemyChasing = false;
                currentState = STATE_GAMEOVER;
                Sleep(500);
                continue;
            }
            
            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
                colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            }

            if (isVisible(fEnemyX, fEnemyY, fPlayerX, fPlayerY)) {
                enemyChasing = true;
            } else {
                enemyChasing = false;
            }

            if (enemyChasing) {
                moveEnemy(&fEnemyX, &fEnemyY, fPlayerX, fPlayerY, fElapsedTime);
            } else {
                fEnemyDirTimer -= fElapsedTime;
                if (fEnemyDirTimer <= 0.0f) {
                    fEnemyDirAngle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159f;
                    fEnemyDirTimer = 0.3f;
                }
                float dx = cosf(fEnemyDirAngle) * fElapsedTime;
                float dy = sinf(fEnemyDirAngle) * fElapsedTime;
                int nx = (int)(fEnemyX + dx);
                int ny = (int)(fEnemyY + dy);
                if ((map[nx * nMapWidth + ny] != '#') && (map[nx * nMapWidth + ny] != '%')) {
                    fEnemyX += dx;
                    fEnemyY += dy;
                }

                }       

            for (int x = 0; x < nScreenWidth; x++) {
                float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / nScreenWidth) * fFOV;

                float fStepSize = 0.1f;
                float fDistanceToWall = 0.0f;

                int nTestX, nTestY;
                bool bHitWall = false;
                bool bBoundary = false;
                bool enemy = false;

                float fEyeX = sinf(fRayAngle);
                float fEyeY = cosf(fRayAngle);

                while (!bHitWall && fDistanceToWall < fDepth) {
                    fDistanceToWall += fStepSize;

                    nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                    nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                    if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
                        bHitWall = true;
                        fDistanceToWall = fDepth;
                    } else {
                        if ((int)fEnemyX == nTestX && (int)fEnemyY == nTestY) {
                            bHitWall = true;
                            enemy = true;
                        }                    
                        if (map[nTestX * nMapWidth + nTestY] == '#') {
                            bHitWall = true;

                            float fBound = 0.01f;
                            for (int tx = 0; tx < 2; tx++) {
                                for (int ty = 0; ty < 2; ty++) {
                                    float vy = (float)nTestY + ty - fPlayerY;
                                    float vx = (float)nTestX + tx - fPlayerX;
                                    float d = sqrtf(vx*vx + vy*vy);
                                    float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                    if (acosf(dot) < fBound)
                                        bBoundary = true;
                                }
                            }
                        }
                    }
                }

                int nCeiling = 0;
                int nFloor = 0;
                int enemyHeight = 0;

                if(enemy){
                float center = nScreenWidth / 2.0f;
                float offset = (x - center) / center;

                float circleFactor = sqrtf(fmaxf(0.0f, 1.0f - offset * offset));

                enemyHeight = (nScreenHeight / fDistanceToWall) * circleFactor;

                nCeiling = (float)(nScreenHeight / 2.0) - enemyHeight;
                nFloor = nScreenHeight - nCeiling;
                }else {
                nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / fDistanceToWall;
                nFloor = nScreenHeight - nCeiling;
                }

                wchar_t nShade = ' ';
                if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588;
                else if (fDistanceToWall < fDepth / 3.0f)   nShade = 0x2593;
                else if (fDistanceToWall < fDepth / 2.0f)   nShade = 0x2592;
                else if (fDistanceToWall < fDepth)          nShade = 0x2591;
                else                                        nShade = ' ';
                
                if(enemy){
                    nShade = '*';
                } 

                if (bBoundary && !enemy)
                    nShade = '|';

                for (int y = 0; y < nScreenHeight; y++) {
                    if (y <= nCeiling){
                        screen[y * nScreenWidth + x] = ' ';
                        colors[y * nScreenWidth + x] = BACKGROUND_BLUE;
                    }
                    else if (y > nCeiling && y <= nFloor){

                        if (enemy){
                            int eyeHeight = nCeiling + (enemyHeight / 2);
                            int eyeOffsetX = 4;
                            int eyeLeft = (nScreenWidth / 2) - eyeOffsetX;
                            int eyeRight = (nScreenWidth / 2) + eyeOffsetX;

                            int smileCenterY = nCeiling + (enemyHeight * 2 / 3);
                            float offset = (x - (nScreenWidth / 2.0f)) / (enemyHeight * 1.2f);
                            int smileY = smileCenterY + (int)(-3.0f * (offset * offset - 1.0f));

                                if (y == eyeHeight && (x == eyeLeft || x == eyeRight)) {
                                    screen[y * nScreenWidth + x] = 'O';
                                    colors[y * nScreenWidth + x] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                                } else if (y == smileY && fabsf(offset) <= 1.0f){
                                    screen[y * nScreenWidth + x] = '-';
                                    colors[y * nScreenWidth + x] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                                }else {
                                    screen[y * nScreenWidth + x] = nShade;
                                    colors[y * nScreenWidth + x] = FOREGROUND_RED | FOREGROUND_INTENSITY;
                                }
                        }
                        else
                        screen[y * nScreenWidth + x] = nShade;
                    }
                    else {
                        float b = 1.0f - ((float)y - nScreenHeight / 2.0f) / (nScreenHeight / 2.0f);
                        if (b < 0.25)      nShade = '#';
                        else if (b < 0.5)  nShade = 'x';
                        else if (b < 0.75) nShade = '.';
                        else if (b < 0.9)  nShade = '-';
                        else              nShade = ' ';
                        screen[y * nScreenWidth + x] = nShade;
                        colors[y * nScreenWidth + x] = BACKGROUND_GREEN;
                    }
                }
            }
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            currentState = STATE_PAUSE;
            
            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            break;

        case STATE_GAMEOVER:

            if (!gameoverMusicPlaying) {
                PlaySound(NULL, 0, 0); 
                PlaySound(TEXT("assets/gameover.wav"), NULL, SND_FILENAME | SND_ASYNC);
                gameoverMusicPlaying = true;
            }

            drawGameOver(screen);


            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
                colors[i] = 0;

                if (screen[i] != L' ') {
                    colors[i] = FOREGROUND_RED | FOREGROUND_INTENSITY;
                }

                int optionY = nScreenHeight / 2 + 2;
                const wchar_t *options[] = {L"[R] Reiniciar Jogo", L"[M] Voltar ao Menu"};
                
                for (int j = 0; j < 2; j++) {
                    int optionX = (nScreenWidth - wcslen(options[j])) / 2;
                    if (i >= (optionY + j*2) * nScreenWidth + optionX && 
                        i < (optionY + j*2) * nScreenWidth + optionX + wcslen(options[j])) {
                        colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                    }
                }
            }

            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);

            static bool keyProcessed = false;
            
            if (GetAsyncKeyState('R') & 0x8000) {
                if (!keyProcessed) {
                    victoryMusicPlaying = false;
                    gameoverMusicPlaying = false;
                    fPlayerX = 25.99f;
                    fPlayerY = 10.70f;
                    fEnemyX = 3.48f;
                    fEnemyY = 16.92f;
                    fPlayerA = 4.0f;
                    currentState = STATE_PLAYING;
                    keyProcessed = true;
                }
            } 
            else if (GetAsyncKeyState('M') & 0x8000) {
                if (!keyProcessed) {
                    currentState = STATE_MENU;
                    keyProcessed = true;
                }
            } 
            else {
                keyProcessed = false;
            }
            break;

        case STATE_PAUSE:
            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
                    screen[i] = ' ';
                    colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                }

                const wchar_t* pauseMsg = L"PAUSE - Pressione ESC para continuar";
                int msgLen = wcslen(pauseMsg);
                int startX = (nScreenWidth / 2) - (msgLen / 2);
                int startY = nScreenHeight / 2;

                for (int i = 0; i < msgLen; i++) {
                    screen[startY * nScreenWidth + startX + i] = pauseMsg[i];
                    colors[startY * nScreenWidth + startX + i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                }

                WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
                WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);

                if (bEscKeyPressed && !bEscKeyPressedPrev) {
                    currentState = STATE_PLAYING;
                    Sleep(100);
                    break;
                }
            break;

        case STATE_WIN:

            if (!victoryMusicPlaying) {
                PlaySound(NULL, 0, 0);
                PlaySound(TEXT("assets/victory.wav"), NULL, SND_FILENAME | SND_ASYNC);
                victoryMusicPlaying = true;
            }    

            drawWin(screen);

            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
                colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
                if (screen[i] == L'#') {
                    colors[i] = BACKGROUND_RED | BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
                }
                colors[i] |= BACKGROUND_BLUE; 
            }

            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);

            if (GetAsyncKeyState('M') & 0x8000) {
                fPlayerX = 25.99f;
                fPlayerY = 10.70f;
                fEnemyX = 3.48f;
                fEnemyY = 16.92f;
                fPlayerA = 4.0f;
                enemyChasing = false;
                currentState = STATE_MENU;
                Sleep(200);
            }
            break;
        }
    }

    free(screen);
    free(colors);
    return 0;
}