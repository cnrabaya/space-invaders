# 👾 Space Invaders

## Motivation
This project was created as a Valentine's greeting for my special person. At the same time, it also helped me brush up on my C++. This Space Invaders game was based on Nick Tasios' Space Invaders from Scratch guide. I simply modified the game to add text animations and a buffer for a personal greeting after game completion.

---

## Controls

| Key         | Action              |
|-------------|---------------------|
| `Enter`     | Start the game      |
| `←` / `→`  | Move player left/right |
| `Space`     | Fire projectile     |
| `Escape`    | Quit                |

---

## Dependencies

| Library | Purpose |
|---------|---------|
| [GLFW](https://www.glfw.org/) | Window creation and keyboard input |
| [GLEW](https://glew.sourceforge.net/) | OpenGL extension loading |
| OpenGL 3.3 Core Profile | GPU texture upload and fullscreen triangle rendering |

---

## Building

### Linux / macOS

Make sure GLFW and GLEW are installed (e.g. via `apt`, `brew`, or from source), then compile:

```bash
g++ -std=c++17 main.cpp -o space_invaders \
    -lGL -lGLEW -lglfw
```

### macOS (with Homebrew)

```bash
g++ -std=c++17 main.cpp -o space_invaders \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -lGLEW -lglfw \
    -framework OpenGL
```

### Windows (MinGW)

```bash
g++ -std=c++17 main.cpp -o space_invaders.exe \
    -lglew32 -lglfw3 -lopengl32
```

---

## Customizing the Narrative

You can include a special message at the end of the game. The text is defined in the `PAGE SETUP` section of `main.cpp`. There are 4 pages (indices 0–3):

- **Page 0** — Intro text shown after all aliens are destroyed
- **Page 1** — The choice prompt (ends with the YES/NO alien targets appearing)
- **Page 2** — Shown if the player shoots YES
- **Page 3** — Shown if the player shoots NO (game exits after this page)

To add or edit text, populate each page like so:

```cpp
msg_animation->pages[0].num_lines = 3;
msg_animation->pages[0].lines = new const char*[3];
msg_animation->pages[0].lines[0] = "LINE ONE HERE";      // max ~32 chars
msg_animation->pages[0].lines[1] = "LINE TWO HERE";
msg_animation->pages[0].lines[2] = "LINE THREE HERE";
msg_animation->pages[0].display_time = 3.0f;             // seconds before advancing
```

---

## Configuration Constants

| Constant | Default | Description |
|----------|---------|-------------|
| `buffer_width` | 224 | Internal render resolution (width) |
| `buffer_height` | 256 | Internal render resolution (height) |
| `GAME_MAX_PROJECTILES` | 128 | Maximum simultaneous projectiles |
| `NUM_PAGES` | 4 | Number of narrative text pages |
| `player_speed` | 60.0f | Player movement speed (pixels/sec) |
| `type_speed` | 13.0f | Typewriter characters per second |

---

## License

This project is provided as-is for educational and personal use.