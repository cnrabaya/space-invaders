# 👾 Space Invaders

## Motivation
This project was created as a Valentine's greeting for my special person. At the same time, it also helped me brush up on my C++. This Space Invaders game was based on Nick Tasios' Space Invaders from Scratch guide. I simply modified the game to add text animations and a buffer for a personal greeting after game completion.

---

## Features

- **Software Pixel Buffer** — All rendering is done by writing to a `uint32_t` buffer that gets uploaded to a GPU texture each frame via OpenGL.
- **Sprite System** — Custom 1-bit sprites for aliens, the player, projectiles, a title card, and a death animation. Supports scaled drawing.
- **Animated Aliens** — Two-frame flip-book animation for aliens. Aliens take two hits to destroy, showing a death sprite on the first hit.
- **Typewriter Narrative** — After all aliens are cleared, a multi-page text animation plays using a character-by-character typewriter effect.
- **Branching Ending** — At the end of the narrative, two alien targets appear labeled YES and NO. Shooting one or the other determines the ending.
- **Custom Pixel Font** — A built-in 5×7 spritesheet font supports full uppercase ASCII text and numbers.
- **Title Screen** — A startup screen with a pixel-art "SPACE INVADERS" logo and control hints.

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

## Project Structure

```
.
└── main.cpp          # Entire game: rendering, input, sprites, game logic, narrative
```

The project is intentionally contained in a single file. Key sections are clearly delimited with block comments:

- `PACKAGE INITIALIZATION` — GLFW/GLEW/OpenGL setup
- `BUFFER SETUP` — Shader compilation, texture creation
- `SPRITES` — All sprite and animation data
- `PAGE SETUP` — Narrative text page definitions
- `GAME SETTINGS` — Alien grid layout and player initialization
- `GAME LOOP` — Per-frame rendering, input handling, collision detection

---

## Customizing the Narrative

The story text is defined in the `PAGE SETUP` section of `main.cpp`. There are 4 pages (indices 0–3):

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