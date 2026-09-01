#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <queue>

using std::vector;
using namespace sf;

/*
Standard Minesweeper boards

Beginner = 9x9 (10 mines)
Intermediate = 16x16 (40 mines)
Expert = 30x16 (99 mines)
*/

const float TILE_SIZE = 32.f;

struct Tile{
    bool hidden = true;
    bool isMine = false;
    bool isFlagged = false;
    int adjacentMines = 0;
};

struct Position{
    int row;
    int col;

    Position(int makeRow, int makeCol){
        row = makeRow;
        col = makeCol;
    }
};

enum class GameState
{
    MainMenu,
    CustomSetup,
    Playing,
    GameOver
};

vector<vector<Tile>> SetGameBoard(int numMines, int rows, int cols);

void PrintBoard(const vector<vector<Tile>>& Board);
void Reveal(vector<vector<Tile>>& Board, Position startPos, int& remainingSafeTiles, bool firstClick);

void DrawBoard(RenderWindow& window, vector<vector<Tile>>& Board, const Texture& tileTexture);
void SetButtonParameters(RectangleShape& button);
void StartGame(vector<vector<Tile>>& Board, View& view, int mines, int rows, int cols, bool& mineHit, bool& firstClick);
void RevealBoard(vector<vector<Tile>>& Board);
void MoveMines(vector<vector<Tile>>& Board, const int& row, const int& col);

float getBoardSizeX(vector<vector<Tile>>& Board);
float getBoardSizeY(vector<vector<Tile>>& Board);
bool Contains(const vector<Position>& arr, const Position& key);

int main(){
    Texture tileTexture("assets/Sprites.png");
    Font font("assets/RubikDistressed-Regular.ttf");

    GameState state = GameState::MainMenu;
    vector<vector<Tile>> GameBoard;
    
    RenderWindow window(VideoMode({1200,1000}), "Minesweeper", Style::Default);
    window.setFramerateLimit(60);

    // Main Menu shapes
    RectangleShape beginnerButton({300.f, 60.f});
    RectangleShape intermediateButton({300.f, 60.f});
    RectangleShape expertButton({300.f, 60.f});
    RectangleShape customButton({300.f, 60.f});

    beginnerButton.setPosition({450.f, 250.f});
    intermediateButton.setPosition({450.f, 330.f});
    expertButton.setPosition({450.f, 410.f});
    customButton.setPosition({450.f, 490.f});

    SetButtonParameters(beginnerButton);
    SetButtonParameters(intermediateButton);
    SetButtonParameters(expertButton);
    SetButtonParameters(customButton);

    Text title(font);
    title.setString("MINESWEEPER");
    title.setCharacterSize(100);
    title.setFillColor(Color::White);
    title.setPosition({230.f, 100.f});

    Text beginnerText(font);
    Text intermediateText(font);
    Text expertText(font);
    Text customText(font);

    beginnerText.setPosition({530.f, 263.f});
    intermediateText.setPosition({500.f, 343.f});
    expertText.setPosition({550.f, 423.f});
    customText.setPosition({550.f, 503.f});

    beginnerText.setString("Beginner");
    intermediateText.setString("Intermediate");
    expertText.setString("Expert");
    customText.setString("Custom");

    // Creates the hud rectangle
    RectangleShape hud;
    hud.setSize({static_cast<float>(window.getSize().x), 50.f});
    hud.setPosition({0.f, 0.f});
    hud.setFillColor(Color(120, 100, 80));

    // Creates the text for the timer
    Text timerText(font);
    timerText.setPosition({0.f, 5.f});

    // Creates the text for the mine counter
    Text minesText(font);
    minesText.setPosition({300.f, 5.f});

    // Creates the text for the Game Over screen
    Text gameOverText(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(100);
    gameOverText.setFillColor(Color::Red);
    gameOverText.setPosition({300.f, 30.f});

    Text victoryText(font);
    victoryText.setString("VICTORY");
    victoryText.setCharacterSize(100);
    victoryText.setFillColor(Color::Cyan);
    victoryText.setPosition({380.f, 30.f});

    // Creates the main menu button on the game over screen
    RectangleShape backToMenuButton({300.f, 60.f});
    backToMenuButton.setPosition({450.f, 160.f});
    SetButtonParameters(backToMenuButton);

    Text backToMenuText(font);
    backToMenuText.setString("Main Menu");
    backToMenuText.setPosition({515.f, 173.f});

    // Creates the camera
    View view({0.f, 0.f}, {750.f, 750.f});
    
    const float MOVESPEED = 5.f;
    const float ZOOM_IN = 0.98f;
    const float ZOOM_OUT = 1.02f;
    const float MIN_VIEW_WIDTH = 400.f;
    const float MAX_VIEW_WIDTH = 3000.f;
    
    int minesRemaining = 0;
    int remainingSafeTiles = 0;
    bool firstClick = true;
    bool mineHit = false;

    Clock gameClock;
    gameClock.reset();
    while(window.isOpen()){
        while(const std::optional event = window.pollEvent()){
            if(event->is<Event::Closed>()) window.close();
            
            if(const auto* pressed = event->getIf<Event::MouseButtonPressed>()){
                if(state == GameState::MainMenu){
                    window.setView(window.getDefaultView());
                    if (pressed->button == sf::Mouse::Button::Left){
                        Vector2f mousePos = window.mapPixelToCoords(pressed->position);

                        if (beginnerButton.getGlobalBounds().contains(mousePos)){
                            StartGame(GameBoard, view, 10, 9, 9, mineHit, firstClick);
                            state = GameState::Playing;
                            minesRemaining = 10;
                            remainingSafeTiles = 9 * 9 - 10;
                        }
                        else if (intermediateButton.getGlobalBounds().contains(mousePos)){
                            StartGame(GameBoard, view, 40, 16, 16, mineHit, firstClick);
                            state = GameState::Playing;
                            minesRemaining = 40;
                            remainingSafeTiles = 16 * 16 - 40;
                        }
                        else if (expertButton.getGlobalBounds().contains(mousePos)){
                            StartGame(GameBoard, view, 99, 16, 30, mineHit, firstClick);
                            state = GameState::Playing;
                            minesRemaining = 99;
                            remainingSafeTiles = 16 * 30 - 99;
                        }
                        else if (customButton.getGlobalBounds().contains(mousePos)){
                            //state = GameState::CustomSetup;
                        }
                    }
                }
                else if(state == GameState::Playing){
                    window.setView(view);

                    // Gets the coordinates of the mouse click            
                    Vector2f worldPos = window.mapPixelToCoords(pressed->position);

                    int col = static_cast<int>(worldPos.x / TILE_SIZE);
                    int row = static_cast<int>(worldPos.y / TILE_SIZE);
    
                    // Reveals tiles with left click
                    if(pressed->button == Mouse::Button::Left){
                        if (row >= 0 && row < GameBoard.size() && col >= 0 && col < GameBoard[row].size()){
                            Reveal(GameBoard, Position(row, col), remainingSafeTiles, firstClick);
                            if(firstClick && !GameBoard[row][col].isFlagged){
                                gameClock.restart();
                                firstClick = false;
                            }
                            else if(GameBoard[row][col].isMine && !GameBoard[row][col].isFlagged){
                                mineHit = true;
                                state = GameState::GameOver;
                                RevealBoard(GameBoard);
                            }
                        }
                    }
    
                    // Flags tiles with right click
                    if(pressed->button == Mouse::Button::Right){
                        if (row >= 0 && row < GameBoard.size() && col >= 0 && col < GameBoard[row].size()){
                            if(GameBoard[row][col].hidden){
                                if(!GameBoard[row][col].isFlagged){
                                    GameBoard[row][col].isFlagged = true;
                                    minesRemaining--;
                                }
                                else{
                                    GameBoard[row][col].isFlagged = false;
                                    minesRemaining++;
                                }
                            }
                        }
                    }
                }
                else if(state == GameState::GameOver){
                    window.setView(window.getDefaultView());
                    gameClock.reset();
                    if(pressed->button == Mouse::Button::Left){
                        Vector2f mousePos = window.mapPixelToCoords(pressed->position);
                        if(backToMenuButton.getGlobalBounds().contains(mousePos)){
                            state = GameState::MainMenu;
                        }
                    }
                }
            }
        }

        // Clear the window before drawing anything in the loop
        window.clear(Color(181,148,103));

        // Draw the main menu elements
        if (state == GameState::MainMenu){
            window.setView(window.getDefaultView());

            window.draw(title);

            window.draw(beginnerButton);
            window.draw(intermediateButton);
            window.draw(expertButton);
            window.draw(customButton);

            window.draw(beginnerText);
            window.draw(intermediateText);
            window.draw(expertText);
            window.draw(customText);
        }

        if(state == GameState::Playing){
            // Set the view for handling game events
            window.setView(view);
    
            // Controls the camera
            if(Keyboard::isKeyPressed(Keyboard::Key::W)) view.move({0.f,-MOVESPEED});
            if(Keyboard::isKeyPressed(Keyboard::Key::A)) view.move({-MOVESPEED,0.f});
            if(Keyboard::isKeyPressed(Keyboard::Key::S)) view.move({0.f,MOVESPEED});
            if(Keyboard::isKeyPressed(Keyboard::Key::D)) view.move({MOVESPEED,0.f});
            
            if(Keyboard::isKeyPressed(Keyboard::Key::Up)){
                if (view.getSize().x > MIN_VIEW_WIDTH) view.zoom(ZOOM_IN);
            }
    
            if(Keyboard::isKeyPressed(Keyboard::Key::Down)){
                if (view.getSize().x < MAX_VIEW_WIDTH) view.zoom(ZOOM_OUT);
            }
    
            int seconds = static_cast<int>(gameClock.getElapsedTime().asSeconds());
            timerText.setString("Time: " + std::to_string(seconds));
            minesText.setString("Mines: " + std::to_string(minesRemaining));
            
            DrawBoard(window, GameBoard, tileTexture);
            
            // Set the view for handling UI elements
            window.setView(window.getDefaultView());
    
            // Draw hud elements
            window.draw(hud);
            window.draw(timerText);
            window.draw(minesText);

            if(remainingSafeTiles == 0){
                RevealBoard(GameBoard);
                state = GameState::GameOver;
            }
        }

        if(state == GameState::GameOver){
            window.setView(view);
            view.setCenter({getBoardSizeX(GameBoard) / 2.f, getBoardSizeY(GameBoard) / 2.f});

            // Allows for zoom on the game over screen
            if(Keyboard::isKeyPressed(Keyboard::Key::Up)){
                if (view.getSize().x > MIN_VIEW_WIDTH) view.zoom(ZOOM_IN);
            }
            
            if(Keyboard::isKeyPressed(Keyboard::Key::Down)){
                if (view.getSize().x < MAX_VIEW_WIDTH) view.zoom(ZOOM_OUT);
            }
            DrawBoard(window, GameBoard, tileTexture);
            
            window.setView(window.getDefaultView());

            if(mineHit) window.draw(gameOverText);
            else window.draw(victoryText);

            window.draw(backToMenuButton);
            window.draw(backToMenuText);
        }
        window.display();
    }
    return 0;
}

vector<vector<Tile>> SetGameBoard(int numMines, int rows, int cols){
    int minesPlaced = 0;
    
    std::random_device rd;
    std::mt19937 gen(rd());

    vector<vector<Tile>> GameBoard(rows, vector<Tile>(cols));

    std::uniform_int_distribution<int> rowDist(0, rows - 1);
    std::uniform_int_distribution<int> colDist(0, cols - 1);
    while(minesPlaced != numMines){
        int row = rowDist(gen);
        int col = colDist(gen);

        if(GameBoard[row][col].isMine == true){
            continue;
        } else {
            GameBoard[row][col].isMine = true;
            minesPlaced++;

            for(int i = -1; i <= 1; ++i){
                for(int j = -1; j <= 1; ++j){
                    int neighborRow = row+i;
                    int neighborCol = col+j;

                    if(neighborRow < 0 || neighborCol < 0 || neighborRow >= rows || neighborCol >= cols) continue;
                    
                    //if(GameBoard[neighborRow][neighborCol].isMine == false) {
                    //}
                    GameBoard[neighborRow][neighborCol].adjacentMines++;
                }
            }
        }
    }
    return GameBoard;
}

float getBoardSizeX(vector<vector<Tile>>& Board){
    return Board[0].size() * TILE_SIZE;
}

float getBoardSizeY(vector<vector<Tile>>& Board){
    return Board.size() * TILE_SIZE;
}

void Reveal(vector<vector<Tile>>& Board, Position startPos, int& remainingSafeTiles, bool firstClick){
    if(Board[startPos.row][startPos.col].isMine && !firstClick) return;
    if(!Board[startPos.row][startPos.col].hidden) return;
    if(Board[startPos.row][startPos.col].isFlagged) return;

    const int rows = Board.size();
    const int cols = Board[0].size();

    if(firstClick){
        MoveMines(Board, startPos.row, startPos.col);
    }

    Board[startPos.row][startPos.col].hidden = false;
    remainingSafeTiles--;

    std::queue<Position> revealQueue;
    revealQueue.push(startPos);

    // Reveal the clicked tiles neighbors with zero adjecent mines
    while(!revealQueue.empty()){
        Position current = revealQueue.front();
        revealQueue.pop();

        if(Board[current.row][current.col].adjacentMines != 0) continue;
        
        for(int i = -1; i <= 1; ++i){
            for(int j = -1; j <=1; ++j){
                if (i == 0 && j == 0) continue;
                Position neighborPos(current.row+i, current.col+j);
                if(neighborPos.row < 0 || neighborPos.col < 0 || neighborPos.row >= rows || neighborPos.col >= cols) continue;
                if(Board[neighborPos.row][neighborPos.col].isFlagged || 
                    Board[neighborPos.row][neighborPos.col].isMine || 
                    !Board[neighborPos.row][neighborPos.col].hidden) continue;

                Board[neighborPos.row][neighborPos.col].hidden = false;
                remainingSafeTiles--;

                if(Board[neighborPos.row][neighborPos.col].adjacentMines == 0){
                    revealQueue.push(neighborPos);
                }
            }
        }
    }
}

void MoveMines(vector<vector<Tile>>& Board, const int& row, const int& col){
    const int numRows = Board.size();
    const int numCols = Board[0].size();

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> rowDist(0, numRows - 1);
    std::uniform_int_distribution<int> colDist(0, numCols - 1);

    int numReplaceMines = 0;
    vector<Position> exclusionZone;

    for(int i = -2; i <= 2; ++i){
        for(int j = -2; j <= 2; ++j){
            int checkRow = row+i;
            int checkCol = col+j;

            if(checkRow < 0 || checkCol < 0 || checkRow >= numRows || checkCol >= numCols) continue;

            exclusionZone.push_back(Position(checkRow, checkCol));

            if(Board[checkRow][checkCol].isMine){
                Board[checkRow][checkCol].isMine = false;
                numReplaceMines++;
                
                for(int i = -1; i <= 1; ++i){
                    for(int j = -1; j <= 1; ++j){
                        int neighborRow = checkRow+i;
                        int neighborCol = checkCol+j;
            
                        if(neighborRow < 0 || neighborCol < 0 || neighborRow >= numRows || neighborCol >= numCols) continue;
                        
                        Board[neighborRow][neighborCol].adjacentMines--;
                    }
                }
            }
        }
    }

    while(numReplaceMines > 0){
        int newRow = rowDist(gen);
        int newCol = colDist(gen);

        if(Contains(exclusionZone, Position(newRow, newCol))) continue;

        if(!Board[newRow][newCol].isMine){
            Board[newRow][newCol].isMine = true;
            numReplaceMines--;

            for(int i = -1; i <= 1; ++i){
                for(int j = -1; j <= 1; ++j){
                    int neighborRow = newRow+i;
                    int neighborCol = newCol+j;

                    if(neighborRow < 0 || neighborCol < 0 || neighborRow >= numRows || neighborCol >= numCols) continue;
                    
                    Board[neighborRow][neighborCol].adjacentMines++;
                }
            }
        }
    }
}

void PrintBoard(const vector<vector<Tile>>& Board){
    for(int i = 0; i < Board.size(); ++i){
        for( int j = 0; j < Board[i].size(); ++j){
            if(Board[i][j].isMine == true){
                std::cout << "* ";
            } else{
                std::cout << Board[i][j].adjacentMines << " ";
            }
        }
        std::cout << "\n";
    }
}

void DrawBoard(RenderWindow& window, vector<vector<Tile>>& Board, const Texture& tileTexture){
    for(int row = 0; row < Board.size(); ++row){
        for(int col = 0; col < Board[row].size(); ++col){
            // Determine which sprite to draw
            int spriteIndex;
            if(Board[row][col].isFlagged){
                spriteIndex = 1;
            } else if(Board[row][col].hidden){
                spriteIndex = 0;
            } else if(Board[row][col].isMine){
                spriteIndex = 2;
            } else{
                spriteIndex = 3 + Board[row][col].adjacentMines;
            }

            Sprite sprite(tileTexture);

            sprite.setTextureRect(IntRect({static_cast<int>(spriteIndex * TILE_SIZE), 0}, {static_cast<int>(TILE_SIZE), static_cast<int>(TILE_SIZE)}));
            sprite.setPosition({col*TILE_SIZE , row*TILE_SIZE});

            window.draw(sprite);
        }
    }
}

void SetButtonParameters(RectangleShape& button){
    button.setFillColor(Color(160, 140, 120));
    button.setOutlineThickness(2.f);
    button.setOutlineColor(Color::Black);
}

void StartGame(vector<vector<Tile>>& Board, View& view, int mines, int rows, int cols, bool& mineHit, bool& firstClick){
    Board = SetGameBoard(mines, rows, cols);
    mineHit = false;
    firstClick = true;
    view.setCenter({getBoardSizeX(Board) / 2.f, getBoardSizeY(Board) / 2.f});
}

void RevealBoard(vector<vector<Tile>>& Board){
    for(int row = 0; row < Board.size(); ++row){
        for(int col = 0; col < Board[row].size(); ++col){
            if(Board[row][col].isFlagged && !Board[row][col].isMine) Board[row][col].isFlagged = false;
            Board[row][col].hidden = false;
        }
    }
}

bool Contains(const vector<Position>& arr, const Position& key){
    for(const Position& element : arr){
        if(element.row == key.row && element.col == key.col) return true;
    }
    return false;
}