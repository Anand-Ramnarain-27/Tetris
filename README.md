# 🎮 Tetris Clone (C++ with ncurses)

This is one of the **first games I built with C++** during my **first year of university**—a fully functional, terminal-based **Tetris clone** using the `ncurses` library for rendering. As an early project in my game development journey, this helped me dive deep into concepts like 2D game loops, user input, and game state management.

While it might seem like a simple game on the surface, it was an incredibly rewarding challenge that pushed me to understand and implement everything from collision detection to score tracking and real-time input handling.

---

## 🧩 Features

- **Classic Tetris Mechanics**
  - Falling tetrominoes, line clearing, scoring, level progression
- **Ghost Piece** (preview of where the piece will land)
- **Piece Hold System** (swap and store a piece)
- **Next Piece Preview**
- **Pause and Resume**
- **High Score Saving** with persistent leaderboard (`highscores.txt`)
- **Hard Drop** and **Soft Drop**
- **Rotations and Wall Kicks**

---

## 🧠 What I Learned

This project introduced me to:
- Game state loops in C++
- Real-time input using `ncurses`
- 2D arrays for board and shape representation
- Basic physics (gravity/fall speed logic)
- Collision detection
- File I/O for saving high scores
- Clean class architecture (`Tetris` class encapsulating core logic)

---

## 🛠️ Tech Stack

- **Language**: C++
- **Rendering/Input**: [`ncurses`](https://invisible-island.net/ncurses/)
- **Compilation**: Uses `g++` and standard Unix tools

---

## 💾 How to Build & Run

### 🔧 Prerequisites
- Linux or WSL (or any Unix-like terminal)
- `g++` compiler
- `ncurses` installed

### ⚙️ Compile:
```bash
g++ tetris.cpp -o tetris -lncurses
```

### ▶️ Run:
```bash
./tetris
```

---

## 📸 Screenshots

_You can include terminal screenshots or GIFs here showcasing gameplay, hold/preview pieces, ghost piece, and high score screen._

---

## 📁 File Overview

```bash
.
├── tetris.cpp           # Complete game source code
└── highscores.txt       # Auto-generated file to store high scores
```

---

## 🏆 High Score System

At the end of each session, if your score qualifies as a top 10 high score, you're prompted to enter your name. Scores are saved to `highscores.txt` and persist between sessions.

---

## 📌 Controls

| Key         | Action            |
|-------------|-------------------|
| Arrow Keys  | Move/Rotate       |
| Space       | Hard Drop         |
| C           | Hold Piece        |
| P           | Pause/Resume      |
| Q           | Quit Game         |

---

## 📜 Final Thoughts

This project holds a special place in my learning journey. It was one of the **first real games I built in C++**, and it taught me not only how to code, but how to **think like a game developer**. From managing game loops to handling user input and polishing gameplay mechanics, every part of this was a hands-on learning experience.

If you're also new to game development and want to build something fun and foundational—**this is a great place to start.**
