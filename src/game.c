#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <stdbool.h>

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

    float fEnemyDirAngle = 0.0f;
    float fEnemyDirTimer = 0.0f;

    typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_PAUSE,
    STATE_WIN
    } GameState;

    GameState currentState = STATE_MENU;

    void drawMenu(wchar_t *screen) {
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
        screen[i] = L' ';

    const wchar_t *title = L"=== MEU JOGO ===";
    const wchar_t *option1 = L"1. Iniciar Jogo";
    const wchar_t *option2 = L"2. Sair";

    int startY = nScreenHeight / 2 - 2;
    int startX = (nScreenWidth - wcslen(title)) / 2;

    swprintf(&screen[startY * nScreenWidth + startX], L"%ls", title);
    swprintf(&screen[(startY + 1) * nScreenWidth + startX], L"%ls", option1);
    swprintf(&screen[(startY + 2) * nScreenWidth + startX], L"%ls", option2);

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

    LARGE_INTEGER frequency, t1, t2;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&t1);

    while (1) {
        QueryPerformanceCounter(&t2);
        float fElapsedTime = (float)(t2.QuadPart - t1.QuadPart) / frequency.QuadPart;
        t1 = t2;
        
        switch (currentState)
        {
        case STATE_MENU:
            drawMenu(screen);

            for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
            colors[i] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            }
            WriteConsoleOutputAttribute(hConsole, colors, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, (COORD){0, 0}, &dwBytesWritten);

            if (GetAsyncKeyState('1') & 0x8000) {
            currentState = STATE_PLAYING;
            Sleep(200);
            } else if (GetAsyncKeyState('2') & 0x8000) {
                free(screen);
                free(colors);
                return 0;
            }
            break;

        case STATE_PLAYING:
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

            if ((int)fPlayerX == (int)fEnemyX && (int)fPlayerY == (int)fEnemyY) {
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
            break;

        case STATE_PAUSE:
            break;

        case STATE_WIN:
            break;
        }
    }

    free(screen);
    free(colors);
    return 0;
}