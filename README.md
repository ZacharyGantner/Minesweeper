To simply play the game:
1. Download the zip from the release
2. Extract the files
3. double click the .exe

To build from the source:
1. SFML 3.1.0 for MinGW 64-bit
2. Extract SFML to a desired location
3. Configure the project to point towards SFML's include and lib folders
4. Build the project
5. Copy the required SFML dlls into the same folder as the executable

Controls:
- Left click to reveal a tile
- Right click to flag a tile
- ASWD to pan the view/camera
- Up/Down arrows to zoom in or out

Customization:
Sprites are 32x32. They can be freely modified on the sprite sheet without changing the code.
The game chooses the sprite to draw based on its position on the sprite sheet, so draw any new sprite 
over the old one. Sprites can be changed to 64x64 or larger, however you must change the TILE_SIZE constant
in the cpp file.
