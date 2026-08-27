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

vector<vector<Tile>> SetGameBoard(int numMines, int rows, int cols);

void PrintBoard(const vector<vector<Tile>>& Board);
void Reveal(vector<vector<Tile>>& Board, Position startPos);

void DrawBoard(RenderWindow& window, vector<vector<Tile>>& Board, const Texture& tileTexture);

float getBoardSizeX(vector<vector<Tile>>& Board);
float getBoardSizeY(vector<vector<Tile>>& Board);

int main(){
    Texture tileTexture("assets/Sprites.png");
    Font font("assets/RubikDistressed-Regular.ttf");

    vector<vector<Tile>> GameBoard = SetGameBoard(40, 16, 16);
    
    RenderWindow window(VideoMode({1200,1000}), "Minesweeper", Style::Default);
    window.setFramerateLimit(60);

    // Creates the hud rectangle
    RectangleShape hud;
    hud.setSize({static_cast<float>(window.getSize().x), 50.f});
    hud.setPosition({0.f, 0.f});
    hud.setFillColor(Color(120, 100, 80));

    // Creates the text for the difficulty selection
    Text difficultyText(font);
    difficultyText.setString("Expert");
    difficultyText.setPosition({500.f, 5.f});

    // Creates the text for the timer
    Text timerText(font);
    timerText.setPosition({0.f, 5.f});

    // Creates the text for the mine counter
    Text minesText(font);
    minesText.setPosition({900.f, 5.f});

    // Creates the camera and centers it on the board
    View view({getBoardSizeX(GameBoard)/2.f, getBoardSizeY(GameBoard)/2.f}, {750.f, 750.f});
    window.setView(view);
    
    const float MOVESPEED = 5.f;
    const float ZOOM_IN = 0.98f;
    const float ZOOM_OUT = 1.02f;
    const float MIN_VIEW_WIDTH = 400.f;
    const float MAX_VIEW_WIDTH = 3000.f;
    
    bool firstClick = true;

    Clock gameClock;
    gameClock.reset();
    while(window.isOpen()){
        // Set the view for handling game events
        window.setView(view);

        while(const std::optional event = window.pollEvent()){
            if(event->is<sf::Event::Closed>()) window.close();
            
            if(const auto* pressed = event->getIf<Event::MouseButtonPressed>()){
                // Gets the coordinates of the mouse click            
                Vector2f worldPos = window.mapPixelToCoords(pressed->position);

                int col = static_cast<int>(worldPos.x / TILE_SIZE);
                int row = static_cast<int>(worldPos.y / TILE_SIZE);

                // Reveals tiles with left click
                if(pressed->button == Mouse::Button::Left){
                    if (row >= 0 && row < GameBoard.size() && col >= 0 && col < GameBoard[row].size()){
                        Reveal(GameBoard, Position(row, col));
                        if(firstClick){
                            gameClock.restart();
                            firstClick = false;
                        }
                    }
                }

                // Flags tiles with right click
                if(pressed->button == Mouse::Button::Right){
                    if (row >= 0 && row < GameBoard.size() && col >= 0 && col < GameBoard[row].size()){
                        if(!GameBoard[row][col].isFlagged){
                            GameBoard[row][col].isFlagged = true;
                        } else{
                            GameBoard[row][col].isFlagged = false;
                        }
                    }
                }
            }
        }
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
        minesText.setString("Mines: ");
        
        window.clear(Color(181,148,103));
        DrawBoard(window, GameBoard, tileTexture);
        
        // Set the view for handling UI elements
        window.setView(window.getDefaultView());

        // Draw hud elements
        window.draw(hud);
        window.draw(timerText);
        window.draw(difficultyText);
        window.draw(minesText);

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
                for(int j = -1; j <=1; ++j){
                    int neighborRow = row+i;
                    int neighborCol = col+j;

                    if(neighborRow < 0 || neighborCol < 0 || neighborRow >= rows || neighborCol >= cols) continue;
                    
                    if(GameBoard[neighborRow][neighborCol].isMine == false) {
                        GameBoard[neighborRow][neighborCol].adjacentMines++;
                    }
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

void Reveal(vector<vector<Tile>>& Board, Position startPos){
    if(Board[startPos.row][startPos.col].isMine) return;
    if(!Board[startPos.row][startPos.col].hidden) return;
    if(Board[startPos.row][startPos.col].isFlagged) return;

    const int rows = Board.size();
    const int cols = Board[0].size();

    Board[startPos.row][startPos.col].hidden = false;

    std::queue<Position> revealQueue;
    revealQueue.push(startPos);

    while(!revealQueue.empty()){
        Position current = revealQueue.front();
        revealQueue.pop();

        if(Board[current.row][current.col].adjacentMines != 0) continue;
        
        for(int i = -1; i <= 1; ++i){
            for(int j = -1; j <=1; ++j){
                if (i == 0 && j == 0) continue;
                Position neighborPos(current.row+i, current.col+j);
                if(neighborPos.row < 0 || neighborPos.col < 0 || neighborPos.row >= rows || neighborPos.col >= cols) continue;                    if(Board[neighborPos.row][neighborPos.col].isFlagged || 
                    Board[neighborPos.row][neighborPos.col].isMine || 
                    !Board[neighborPos.row][neighborPos.col].hidden) continue;

                Board[neighborPos.row][neighborPos.col].hidden = false;

                if(Board[neighborPos.row][neighborPos.col].adjacentMines == 0){
                    revealQueue.push(neighborPos);
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