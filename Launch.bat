@echo off

g++ -std=c++17 src/Minesweeper.cpp -I sfml/include -L sfml/lib -lsfml-graphics -lsfml-window -lsfml-system -o Minesweeper.exe

set PATH=%CD%\sfml\bin;%PATH%

cmd /k Minesweeper.exe
pause