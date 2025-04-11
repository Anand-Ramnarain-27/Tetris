#include <iostream>
#include <vector>
#include <ncurses.h>
#include <unistd.h>
#include <random>
#include <fstream>
#include <algorithm>
#include <string>

using namespace std;

const int WIDTH = 10;
const int HEIGHT = 20;
const int PREVIEW_WIDTH = 6;
const int HOLD_WIDTH = 6;
const vector<vector<vector<int>>> TETROMINOES = {
    {{1,1,1,1}},
    {{1,1},
     {1,1}},
    {{0,1,0},
     {1,1,1}},
    {{0,0,1},
     {1,1,1}},
    {{1,0,0},
     {1,1,1}},
    {{0,1,1},
     {1,1,0}},
    {{1,1,0},
     {0,1,1}}
};

const int COLOR_I = 1;
const int COLOR_O = 2;
const int COLOR_T = 3;
const int COLOR_L = 4;
const int COLOR_J = 5;
const int COLOR_S = 6;
const int COLOR_Z = 7;
const int GHOST_COLOR = 8;

class Tetris {
private:
    vector<vector<int>> board;
    vector<vector<int>> currentPiece;
    vector<vector<int>> nextPiece;
    vector<vector<int>> holdPiece;
    bool canHold;
    int currentX, currentY;
    int currentColor;
    int nextColor;
    int holdColor;
    int score;
    int level;
    int linesCleared;
    double fallSpeed;
    double fallTime;
    time_t lastFall;
    bool gameOver;
    bool paused;
    vector<pair<string, int>> highScores;

    void initColors() {
        start_color();
        init_pair(COLOR_I, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_O, COLOR_YELLOW, COLOR_BLACK);
        init_pair(COLOR_T, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(COLOR_L, COLOR_WHITE, COLOR_BLACK); 
        init_pair(COLOR_J, COLOR_BLUE, COLOR_BLACK);
        init_pair(COLOR_S, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_Z, COLOR_RED, COLOR_BLACK);
        init_pair(GHOST_COLOR, COLOR_BLACK, COLOR_WHITE);
    }

    int getPieceColor(int pieceIndex) {
        switch(pieceIndex) {
            case 0: return COLOR_I;
            case 1: return COLOR_O;
            case 2: return COLOR_T;
            case 3: return COLOR_L;
            case 4: return COLOR_J;
            case 5: return COLOR_S;
            case 6: return COLOR_Z;
            default: return 0;
        }
    }

    void newPiece() {
        currentPiece = nextPiece;
        currentColor = nextColor;
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, TETROMINOES.size()-1);
        int nextIndex = dist(gen);
        nextPiece = TETROMINOES[nextIndex];
        nextColor = getPieceColor(nextIndex);
        
        currentX = WIDTH / 2 - currentPiece[0].size() / 2;
        currentY = 0;
        canHold = true;
        
        if (checkCollision()) {
            gameOver = true;
        }
    }

    void loadHighScores() {
        highScores.clear();
        ifstream file("highscores.txt");
        if (file.is_open()) {
            string name;
            int score;
            while (file >> name >> score) {
                highScores.emplace_back(name, score);
            }
            file.close();
        }
        sort(highScores.begin(), highScores.end(), 
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second > b.second;
            });
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
    }

    void saveHighScores() {
        ofstream file("highscores.txt");
        if (file.is_open()) {
            for (const auto& entry : highScores) {
                file << entry.first << " " << entry.second << "\n";
            }
            file.close();
        }
    }

    void addHighScore(const string& name, int score) {
        highScores.emplace_back(name, score);
        sort(highScores.begin(), highScores.end(), 
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second > b.second;
            });
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
        saveHighScores();
    }

    bool checkCollision() {
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x] == 0) continue;
                
                int newX = currentX + x;
                int newY = currentY + y;
                
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT) {
                    return true;
                }
                
                if (newY >= 0 && board[newY][newX] != 0) {
                    return true;
                }
            }
        }
        return false;
    }

    void mergePiece() {
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                if (currentPiece[y][x] != 0) {
                    board[currentY + y][currentX + x] = currentColor;
                }
            }
        }
    }

    void clearLines() {
        int linesRemoved = 0;
        for (int y = HEIGHT - 1; y >= 0; y--) {
            bool lineComplete = true;
            for (int x = 0; x < WIDTH; x++) {
                if (board[y][x] == 0) {
                    lineComplete = false;
                    break;
                }
            }
            
            if (lineComplete) {
                linesRemoved++;
                for (int yy = y; yy > 0; yy--) {
                    for (int x = 0; x < WIDTH; x++) {
                        board[yy][x] = board[yy-1][x];
                    }
                }
                for (int x = 0; x < WIDTH; x++) {
                    board[0][x] = 0;
                }
                y++;
            }
        }

        if (linesRemoved > 0) {
            linesCleared += linesRemoved;
            switch(linesRemoved) {
                case 1: score += 100 * level; break;
                case 2: score += 300 * level; break;
                case 3: score += 500 * level; break;
                case 4: score += 800 * level; break;
            }
            level = 1 + linesCleared / 10;
            fallSpeed = max(0.05, 1.0 - (level * 0.05));
        }
    }

    void rotatePiece() {
        vector<vector<int>> rotated(currentPiece[0].size(), vector<int>(currentPiece.size()));
        for (int y = 0; y < currentPiece.size(); y++) {
            for (int x = 0; x < currentPiece[y].size(); x++) {
                rotated[x][currentPiece.size() - 1 - y] = currentPiece[y][x];
            }
        }
        
        vector<vector<int>> oldPiece = currentPiece;
        currentPiece = rotated;
        
        if (checkCollision()) {
            currentX--;
            if (checkCollision()) {
                currentX += 2;
                if (checkCollision()) {
                    currentX--;
                    currentY--;
                    if (checkCollision()) {
                        currentY++;
                        currentPiece = oldPiece;
                    }
                }
            }
        }
    }

    void drawPiece(int x, int y, const vector<vector<int>>& piece, int color, bool ghost = false) {
        for (int py = 0; py < piece.size(); py++) {
            for (int px = 0; px < piece[py].size(); px++) {
                if (piece[py][px] != 0) {
                    if (ghost) {
                        attron(COLOR_PAIR(GHOST_COLOR));
                        mvaddch(y + py, x + px, ' ' | A_REVERSE);
                        attroff(COLOR_PAIR(GHOST_COLOR));
                    } else {
                        attron(COLOR_PAIR(color));
                        mvaddch(y + py, x + px, ' ' | A_REVERSE);
                        attroff(COLOR_PAIR(color));
                    }
                }
            }
        }
    }

    void drawBoard() {
        for (int y = 0; y < HEIGHT; y++) {
            mvaddch(y, -1, '|');
            mvaddch(y, WIDTH, '|');
        }
        for (int x = -1; x <= WIDTH; x++) {
            mvaddch(HEIGHT, x, '-');
        }

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (board[y][x] != 0) {
                    attron(COLOR_PAIR(board[y][x]));
                    mvaddch(y, x, ' ' | A_REVERSE);
                    attroff(COLOR_PAIR(board[y][x]));
                } else {
                    mvaddch(y, x, ' ');
                }
            }
        }
    }

    void drawPreview() {
        for (int y = 0; y < 6; y++) {
            mvaddch(y, WIDTH + 2, '|');
            mvaddch(y, WIDTH + 2 + PREVIEW_WIDTH + 1, '|');
        }
        for (int x = WIDTH + 2; x <= WIDTH + 2 + PREVIEW_WIDTH + 1; x++) {
            mvaddch(0, x, '-');
            mvaddch(6, x, '-');
        }
        mvprintw(0, WIDTH + 4, "NEXT");

        int startX = WIDTH + 2 + (PREVIEW_WIDTH - nextPiece[0].size()) / 2 + 1;
        int startY = 2;
        drawPiece(startX, startY, nextPiece, nextColor);
    }

    void drawHold() {
        for (int y = 0; y < 6; y++) {
            mvaddch(y, WIDTH + 2 + PREVIEW_WIDTH + 3, '|');
            mvaddch(y, WIDTH + 2 + PREVIEW_WIDTH + 3 + HOLD_WIDTH + 1, '|');
        }
        for (int x = WIDTH + 2 + PREVIEW_WIDTH + 3; x <= WIDTH + 2 + PREVIEW_WIDTH + 3 + HOLD_WIDTH + 1; x++) {
            mvaddch(0, x, '-');
            mvaddch(6, x, '-');
        }
        mvprintw(0, WIDTH + 2 + PREVIEW_WIDTH + 3 + 2, "HOLD");

        if (!holdPiece.empty()) {
            int startX = WIDTH + 2 + PREVIEW_WIDTH + 3 + (HOLD_WIDTH - holdPiece[0].size()) / 2 + 1;
            int startY = 2;
            drawPiece(startX, startY, holdPiece, holdColor);
        }
    }

    void drawGhostPiece() {
        int ghostY = currentY;
        while (!checkCollisionAt(currentX, ghostY + 1)) {
            ghostY++;
        }
        
        if (ghostY > currentY) {
            drawPiece(currentX, ghostY, currentPiece, GHOST_COLOR, true);
        }
    }

    bool checkCollisionAt(int x, int y) {
        for (int py = 0; py < currentPiece.size(); py++) {
            for (int px = 0; px < currentPiece[py].size(); px++) {
                if (currentPiece[py][px] == 0) continue;
                
                int newX = x + px;
                int newY = y + py;
                
                if (newX < 0 || newX >= WIDTH || newY >= HEIGHT) {
                    return true;
                }
                
                if (newY >= 0 && board[newY][newX] != 0) {
                    return true;
                }
            }
        }
        return false;
    }

    void drawStats() {
        mvprintw(8, WIDTH + 2, "Score: %d", score);
        mvprintw(9, WIDTH + 2, "Level: %d", level);
        mvprintw(10, WIDTH + 2, "Lines: %d", linesCleared);
    }

    void drawControls() {
        mvprintw(HEIGHT + 2, 0, "Controls: Arrows=Move, Up=Rotate, Space=Hard Drop");
        mvprintw(HEIGHT + 3, 0, "         C=Hold, P=Pause, Q=Quit");
    }

    void drawGameOver() {
        clear();
        mvprintw(HEIGHT/2 - 1, WIDTH/2 - 5, "GAME OVER");
        mvprintw(HEIGHT/2 + 1, WIDTH/2 - 10, "Final Score: %d", score);
        mvprintw(HEIGHT/2 + 3, WIDTH/2 - 12, "Press any key to continue");
        refresh();
        getch();
    }

    void holdCurrentPiece() {
        if (!canHold) return;
        
        if (holdPiece.empty()) {
            holdPiece = currentPiece;
            holdColor = currentColor;
            newPiece();
        } else {
            swap(currentPiece, holdPiece);
            swap(currentColor, holdColor);
            currentX = WIDTH / 2 - currentPiece[0].size() / 2;
            currentY = 0;
        }
        canHold = false;
    }

    void hardDrop() {
        while (!checkCollisionAt(currentX, currentY + 1)) {
            currentY++;
        }
        mergePiece();
        clearLines();
        newPiece();
    }

public:
    Tetris() : board(HEIGHT, vector<int>(WIDTH, 0)), score(0), level(1), 
               linesCleared(0), fallSpeed(1.0), gameOver(false), paused(false) {
        initColors();
        loadHighScores();
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, TETROMINOES.size()-1);
        
        int nextIndex = dist(gen);
        nextPiece = TETROMINOES[nextIndex];
        nextColor = getPieceColor(nextIndex);
        
        newPiece();
        lastFall = time(nullptr);
    }

    void showHighScores() {
        clear();
        mvprintw(1, WIDTH/2 - 6, "HIGH SCORES");
        
        for (size_t i = 0; i < highScores.size(); i++) {
            mvprintw(3 + i, WIDTH/2 - 10, "%2d. %-10s %6d", 
                    i+1, highScores[i].first.c_str(), highScores[i].second);
        }
        
        mvprintw(15, WIDTH/2 - 12, "Press any key to continue");
        refresh();
        getch();
    }

    void draw() {
        clear();
        
        drawBoard();
        drawGhostPiece();
        drawPiece(currentX, currentY, currentPiece, currentColor);
        drawPreview();
        drawHold();
        drawStats();
        drawControls();
        
        if (paused) {
            mvprintw(HEIGHT/2, WIDTH/2 - 3, "PAUSED");
        }
        
        refresh();
    }

    void handleInput(int ch) {
        if (paused && ch != 'p') return;
        
        switch (ch) {
            case KEY_LEFT:
                currentX--;
                if (checkCollision()) currentX++;
                break;
            case KEY_RIGHT:
                currentX++;
                if (checkCollision()) currentX--;
                break;
            case KEY_DOWN:
                currentY++;
                if (checkCollision()) {
                    currentY--;
                    mergePiece();
                    clearLines();
                    newPiece();
                }
                lastFall = time(nullptr); 
                break;
            case KEY_UP:
                rotatePiece();
                break;
            case ' ':
                hardDrop();
                break;
            case 'c':
            case 'C':
                holdCurrentPiece();
                break;
            case 'p':
            case 'P':
                paused = !paused;
                if (!paused) {
                    lastFall = time(nullptr); 
                }
                break;
            case 'q':
            case 'Q':
                gameOver = true;
                break;
        }
    }

    void update() {
        if (gameOver || paused) return;
        
        time_t now = time(nullptr);
        if (difftime(now, lastFall) >= fallSpeed) {
            currentY++;
            if (checkCollision()) {
                currentY--;
                mergePiece();
                clearLines();
                newPiece();
            }
            lastFall = now;
        }
    }

    bool isGameOver() const { return gameOver; }

    void enterHighScore() {
        echo();
        curs_set(1);
        
        clear();
        mvprintw(HEIGHT/2 - 1, WIDTH/2 - 10, "New High Score!");
        mvprintw(HEIGHT/2 + 1, WIDTH/2 - 10, "Score: %d", score);
        mvprintw(HEIGHT/2 + 3, WIDTH/2 - 10, "Enter your name: ");
        
        char name[256];
        getstr(name);
        
        noecho();
        curs_set(0);
        
        addHighScore(name, score);
    }

    bool isHighScore() const {
        if (highScores.size() < 10) return true;
        return score > highScores.back().second;
    }

    void run() {
        while (!gameOver) {
            draw();
            
            int ch = getch();
            if (ch != ERR) {
                handleInput(ch);
            }

            update();
            
            usleep(10000); 
        }
        
        drawGameOver();
        
        if (isHighScore()) {
            enterHighScore();
        }
        
        showHighScores();
    }
};

void showMainMenu() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    clear();
    mvprintw(5, 10, "TETRIS");
    mvprintw(7, 10, "1. Start Game");
    mvprintw(8, 10, "2. High Scores");
    mvprintw(9, 10, "3. Quit");
    mvprintw(11, 10, "Select option: ");
    refresh();
    
    int choice = getch() - '0';
    
    endwin();
    
    if (choice == 1) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        nodelay(stdscr, TRUE);
        
        Tetris game;
        game.run();
        
        endwin();
    } else if (choice == 2) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        
        Tetris game; 
        game.showHighScores();
        
        endwin();
    }
}

int main() {
    while (true) {
        showMainMenu();
        
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        
        clear();
        mvprintw(5, 10, "Play again?");
        mvprintw(7, 10, "Y - Yes");
        mvprintw(8, 10, "N - No");
        refresh();
        
        int ch = tolower(getch());
        endwin();
        
        if (ch != 'y') {
            break;
        }
    }
    
    return 0;
}