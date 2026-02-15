# Maze Runner Game 🏃‍♂️🧱

A strategic Maze Runner vs. Hunters game developed in **C** using the **Raylib** graphical library.

## 🎮 Game Features
- **Procedural Map Generation:** Every game creates a unique maze using DFS algorithms.
- **Smart AI Hunters:** Hunters track the nearest runner using pathfinding logic.
- **Temporary Walls:** Runners can place temporary barriers to block hunters.
- **Random Events:** Green cells trigger random events (Earthquake, Hunter Control, Bonus Walls, etc.).
- **Save/Load System:** Full game state saving capability.

## 🕹️ Controls

| Action | Key |
| :--- | :---: |
| **Move Runner** | `Arrow Keys` (⬆️ ⬇️ ⬅️ ➡️) |
| **Place Wall (Up)** | `W` |
| **Place Wall (Down)** | `S` |
| **Place Wall (Left)** | `A` |
| **Place Wall (Right)** | `D` |
| **Skip Turn** | `0` (Zero) |
| **Control Hunter (Event Mode)** | `Enter` to move, `Tab` to switch hunter |

## 🛠️ How to Compile & Run
This project requires **Raylib**.

### Windows (GCC)
If you have Raylib installed:
```bash
