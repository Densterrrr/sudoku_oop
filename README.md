# sudoku_oop

**Reactive Sudoku Puzzle Analyzer(C++)**
A terminal-based Sudoku game written in C++ that features an interactive hint system to help guide players toward the correct answers.

**How to Play**

1. When the game starts, the puzzle is displayed on the board.
2. Enter a **cell address** using a letter (A–I) for the row and a number (1–9) for the column (e.g., D4).
3. The game will show you hints for that cell before you answer.
4. Choose from the menu options to interact with the board.

**Menu Options**

1. Provide an answer — Enter a number (1–9) for the selected cell. Enter **0** to clear it.
2. Check another cell — Switch to a different cell and view its hints.
3. Reset the puzzle — Wipe all your progress and start from the beginning.
4. Exit — Quit the game.

**Hint System**

Each empty cell tracks three sets of hints:

Row Hints — numbers missing from that row
Column Hints — numbers missing from that column
Box Hints — numbers missing from the 3×3 box
Consolidated Hints — numbers that appear in 'all three' hints, meaning they are safe candidates for that cell

**Class**

Cell - Stores the value, fixed status, and hints for a single cell
SudokuBoard - Manages the 9×9 grid, hint logic, validation, and reset
GameManager - Handles all user input, display, and game flow

## Notes

- Fixed cells (pre-filled by the puzzle) cannot be changed.
- Non-fixed cells can be freely overwritten or cleared by entering `0`.
- Hints automatically update whenever a cell is filled or cleared.
