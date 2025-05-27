Just for fun

## How to Run

1.  **Compile the code:**
    ```bash
    gcc main.c -o main $(sdl2-config --cflags --libs) -lSDL2_image
    ```
2.  **Run the executable:**
    ```bash
    ./main
    ```

Make sure you have SDL2 and SDL2_image development libraries installed on your system.
For example, on Debian/Ubuntu based systems:
```bash
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-image-dev
```
