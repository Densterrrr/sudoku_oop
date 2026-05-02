#include <iostream>
#include <vector>
#include <string>
using namespace std;

// this class is for one cell in the sudoku board
// it stores the number in it, if its locked or not,
// and some arrays that help figure out what numbers can go there

class Cell {
private:
    int value;     // whats in the cell, 0 means its empty
    bool isFixed;  // true if the puzzle already put a number here (cant change it)

    // these track whats missing from the row, col, and box
    vector<int> rowHints, colHints, boxHints;

    // only the numbers that are in all 3 of the above
    vector<int> consolidatedHints;

public:
    // default constructor, just set everything to empty/false
    Cell() : value(0), isFixed(false) {}

    // getters and setters for the value and isFixed properties
    int getValue()          { return value; }
    bool getIsFixed()       { return isFixed; }
    void setValue(int v)    { value = v; }
    void setIsFixed(bool f) { isFixed = f; }

    // SudokuBoard calls these when it recalculates hints
    void setRowHints(vector<int> h) { rowHints = h; }
    void setColHints(vector<int> h) { colHints = h; }
    void setBoxHints(vector<int> h) { boxHints = h; }

    // goes through rowHints and checks if each number also shows up
    // in colHints and boxHints — if yes, add to consolidated
    // basically its the intersection of all 3 arrays
    void refreshConsolidatedHints() {
        consolidatedHints.clear();
        for (int num : rowHints) {
            bool inCol = false, inBox = false;
            for (int c : colHints) if (c == num) inCol = true;
            for (int b : boxHints) if (b == num) inBox = true;
            if (inCol && inBox) consolidatedHints.push_back(num);
        }
    }

    // just returning the hint arrays so other classes can use them
    vector<int> getRowHints()          { return rowHints; }
    vector<int> getColHints()          { return colHints; }
    vector<int> getBoxHints()          { return boxHints; }
    vector<int> getConsolidatedHints() { return consolidatedHints; }
};

// this is the main class that runs the whole board
// it has a 9x9 grid of Cell objects and handles all the logic
// like loading the puzzle, updating hints, checking answers, resetting, etc.
class SudokuBoard {
private:
    Cell grid[9][9];         // the actual board being played
    Cell originalGrid[9][9]; // a copy of the starting state so we can reset

    // looks through a row and finds which numbers are missing
    // then sets those as the rowHints for all empty cells in that row
    void recalculateRowHints(int row) {
        vector<int> used, missing;
        for (int c = 0; c < 9; c++)
            if (grid[row][c].getValue()) used.push_back(grid[row][c].getValue());
        for (int n = 1; n <= 9; n++) {
            bool found = false;
            for (int u : used) if (u == n) found = true;
            if (!found) missing.push_back(n);
        }
        for (int c = 0; c < 9; c++)
            if (!grid[row][c].getValue()) grid[row][c].setRowHints(missing);
    }

    // same thing but for a column instead of a row
    void recalculateColumnHints(int col) {
        vector<int> used, missing;
        for (int r = 0; r < 9; r++)
            if (grid[r][col].getValue()) used.push_back(grid[r][col].getValue());
        for (int n = 1; n <= 9; n++) {
            bool found = false;
            for (int u : used) if (u == n) found = true;
            if (!found) missing.push_back(n);
        }
        for (int r = 0; r < 9; r++)
            if (!grid[r][col].getValue()) grid[r][col].setColHints(missing);
    }

    // finds what 3x3 box the cell is in, checks whats used in that box,
    // then updates box hints for all the empty cells inside it
    void recalculateBoxHints(int row, int col) {
        int sr = (row/3)*3, sc = (col/3)*3; // top-left corner of the box
        vector<int> used, missing;
        for (int r = sr; r < sr+3; r++)
            for (int c = sc; c < sc+3; c++)
                if (grid[r][c].getValue()) used.push_back(grid[r][c].getValue());
        for (int n = 1; n <= 9; n++) {
            bool found = false;
            for (int u : used) if (u == n) found = true;
            if (!found) missing.push_back(n);
        }
        for (int r = sr; r < sr+3; r++)
            for (int c = sc; c < sc+3; c++)
                if (!grid[r][c].getValue()) grid[r][c].setBoxHints(missing);
    }

    // when a cell gets updated, we only redo the hints for the
    // row, column, and box that changed — not the whole board
    // i think this is more efficient than redoing everything
    void reactiveUpdate(int row, int col) {
        recalculateRowHints(row);
        recalculateColumnHints(col);
        recalculateBoxHints(row, col);

        // now we update the consolidated hints for all affected empty cells
        for (int c = 0; c < 9; c++)
            if (!grid[row][c].getValue()) grid[row][c].refreshConsolidatedHints();
        for (int r = 0; r < 9; r++)
            if (!grid[r][col].getValue()) grid[r][col].refreshConsolidatedHints();
        int sr = (row/3)*3, sc = (col/3)*3;
        for (int r = sr; r < sr+3; r++)
            for (int c = sc; c < sc+3; c++)
                if (!grid[r][c].getValue()) grid[r][c].refreshConsolidatedHints();
    }

public:
    // puts the puzzle into the grid, marks which cells are fixed,
    // saves a copy for resetting later, then calculates all the hints
    void loadPuzzle(int puzzle[9][9]) {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++) {
                grid[r][c].setValue(puzzle[r][c]);
                grid[r][c].setIsFixed(puzzle[r][c] != 0);
                originalGrid[r][c] = grid[r][c]; // backup!
            }
        // run reactiveUpdate on every cell so hints are set from the start
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                reactiveUpdate(r, c);
    }

    // turns something like "D4" or "4D" into actual row and column numbers
    // i made it handle both orders so users dont get errors either way
    pair<int,int> translateAddress(string address) {
        if (address.length() < 2) return {-1, -1};
        char a = toupper(address[0]), b = toupper(address[1]);
        int row, col;
        if (isalpha(a) && isdigit(b)) {
            row = a - 'A';
            col = b - '1';
        } else if (isdigit(a) && isalpha(b)) {
            row = b - 'A';
            col = a - '1';
        } else return {-1, -1};
        if (row < 0 || row > 8 || col < 0 || col > 8) return {-1, -1};
        return {row, col};
    }

    // checks if the answer the user gave is in the consolidated hints
    // if consolidated hints is empty for some reason, fall back to
    // just checking if its a duplicate — so the user isnt stuck forever
    bool validateAnswer(int row, int col, int answer) {
        vector<int> hints = grid[row][col].getConsolidatedHints();
        if (!hints.empty()) {
            for (int h : hints)
                if (h == answer) return true;
            return false;
        }
        // fallback check — just make sure its not already somewhere it shouldnt be
        return !isDuplicateInRowColBox(row, col, answer);
    }

    // puts the answer in the cell (only if its not a fixed cell)
    // then calls reactiveUpdate so nearby cells get updated hints
    void assignValue(int row, int col, int answer) {
        if (!grid[row][col].getIsFixed()) {
            grid[row][col].setValue(answer);
            reactiveUpdate(row, col);
        }
    }

    // checks if the number already exists somewhere it shouldnt be
    // skips the cell itself so it doesnt accidentally flag itself
    bool isDuplicateInRowColBox(int row, int col, int answer) {
        // check the row
        for (int c = 0; c < 9; c++)
            if (c != col && grid[row][c].getValue() == answer) return true;
        // check the column
        for (int r = 0; r < 9; r++)
            if (r != row && grid[r][col].getValue() == answer) return true;
        // check the 3x3 box
        int sr = (row/3)*3, sc = (col/3)*3;
        for (int r = sr; r < sr+3; r++)
            for (int c = sc; c < sc+3; c++)
                if (!(r == row && c == col) && grid[r][c].getValue() == answer) return true;
        return false;
    }

    // resets the board back to how it was at the start
    // copies from the backup and redoes all the hints
    void resetPuzzle() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = originalGrid[r][c];
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                reactiveUpdate(r, c);
    }

    // checks if every cell on the board has a value
    // used to detect when the puzzle is fully solved
    bool isBoardComplete() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (grid[r][c].getValue() == 0) return false;
        return true;
    }

    // prints out all 4 hint arrays for whatever cell you pick
    // called from GameManager when the user asks for hints
    void printHints(int row, int col) {
        cout << "\nHints for cell " << (char)('A'+row) << (col+1) << ":" << endl;
        cout << "  Row Hints        : ";
        for (int h : grid[row][col].getRowHints()) cout << h << " ";
        cout << "\n  Column Hints     : ";
        for (int h : grid[row][col].getColHints()) cout << h << " ";
        cout << "\n  Box Hints        : ";
        for (int h : grid[row][col].getBoxHints()) cout << h << " ";
        cout << "\n  Consolidated Hints (safe candidates): ";
        vector<int> ch = grid[row][col].getConsolidatedHints();
        if (ch.empty()) cout << "(none — cell may already be filled or fully constrained)";
        else for (int h : ch) cout << h << " ";
        cout << endl;
    }

    int getCellValue(int r, int c)    { return grid[r][c].getValue(); }
    bool getCellIsFixed(int r, int c) { return grid[r][c].getIsFixed(); }
};

// this class handles everything the user sees and interacts with
// it basically separates the display/input stuff from the board logic
// which i think is what the teacher meant by separation of concerns
class GameManager {
private:
    SudokuBoard board; // the board we're playing on

    // just a small helper to ask the user yes or no
    bool confirm(string message) {
        string r;
        cout << message << " (y/n): ";
        cin >> r;
        return (r == "y" || r == "Y");
    }

public:
    // prints the board to the terminal
    // shows dots for empty cells and actual numbers for filled ones
    void displayBoard() {
        cout << "\n     1 2 3   4 5 6   7 8 9" << endl;
        cout << "   +-------+-------+-------+" << endl;
        for (int r = 0; r < 9; r++) {
            cout << " " << (char)('A'+r) << " |";
            for (int c = 0; c < 9; c++) {
                int val = board.getCellValue(r, c);
                cout << (val ? " " + to_string(val) : " ."); // dot if empty
                if (c == 2 || c == 5 || c == 8) cout << " |";
            }
            cout << " " << (char)('A'+r) << endl;
            if (r == 2 || r == 5)
                cout << "   +-------+-------+-------+" << endl;
        }
        cout << "   +-------+-------+-------+" << endl;
        cout << "     1 2 3   4 5 6   7 8 9\n" << endl;
    }

    // this is where the game starts
    void run() {
        // 0 means empty cell
        int puzzle[9][9] = {
            {5, 3, 4, 0, 7, 8, 9, 1, 2},
            {6, 7, 2, 1, 9, 5, 3, 4, 8},
            {1, 9, 8, 3, 4, 2, 5, 6, 7},
            {8, 5, 9, 7, 6, 1, 4, 2, 3},
            {4, 2, 6, 8, 5, 3, 7, 9, 1},
            {7, 1, 3, 9, 2, 4, 8, 5, 6},
            {9, 6, 1, 5, 3, 7, 2, 8, 4},
            {2, 8, 7, 4, 1, 9, 6, 3, 5},
            {3, 4, 5, 2, 8, 6, 1, 7, 9}
        };

        board.loadPuzzle(puzzle);
        cout << "Welcome! Here is your Sudoku puzzle:" << endl;
        displayBoard();
        menu();
    }

    // the main loop — keeps showing the menu until the user exits
    void menu() {
        string input;
        int choice;

        // lambda that keeps asking until we get a valid cell address
        auto readAddress = [&]() {
            pair<int,int> coords = {-1, -1};
            while (coords.first == -1) {
                cout << "Enter cell address (e.g. D4): ";
                cin >> input;
                coords = board.translateAddress(input);
                if (coords.first == -1)
                    cout << "Invalid address! Use letter A-I and number 1-9 (e.g. D4).\n";
            }
            return coords;
        };

        // ask the user to pick a starting cell and show its hints
        auto coords = readAddress();
        int row = coords.first, col = coords.second;
        board.printHints(row, col);

        do {
            // show the 4 menu options
            cout << "\n--- MENU ---\n"
                 << "1. Provide an answer for this cell\n"
                 << "2. Check another cell\n"
                 << "3. Reset the puzzle\n"
                 << "4. Exit\n"
                 << "Choice: ";

            // if they type something weird, clear cin and try again
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input! Please enter 1-4.\n";
                choice = 0;
                continue;
            }

            if (choice == 1) {
                // cant fill a cell that came with the puzzle
                if (board.getCellIsFixed(row, col)) {
                    cout << "That cell is fixed and cannot be changed!" << endl;
                    continue;
                }

                int answer;
                cout << "Enter your answer (1-9) (0 to clear): ";
                if (!(cin >> answer) || answer < 0 || answer > 9) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input! Enter a number from 0-9.\n";
                    continue;
                }

                // if 0 is entered, clear the cell and show the updated board
                if (answer == 0) {
                    board.assignValue(row, col, 0);
                    cout << "Cell cleared!" << endl;
                    displayBoard();
                    continue;
                }

                // first check if its already used nearby, then check consolidated hints
                if (board.isDuplicateInRowColBox(row, col, answer)) {
                    cout << "Invalid! " << answer
                         << " already exists in the same row, column, or box." << endl;
                } else if (!board.validateAnswer(row, col, answer)) {
                    cout << "Invalid! " << answer
                         << " is not in the consolidated hints for this cell." << endl;
                    cout << "Tip: Use option 2 to view the hints for this cell again." << endl;
                } else {
                    // answer is valid — assign it and show the updated board
                    board.assignValue(row, col, answer);
                    cout << "Answer accepted!" << endl;
                    displayBoard();

                    // check if all cells are filled — if yes, show congratulations and exit
                    if (board.isBoardComplete()) {
                        cout << "Congratulations! Puzzle Solved!" << endl;
                        return; // exit the menu loop and end the game
                    }
                }
            }
            else if (choice == 2) {
                // show the board first so the user knows what theyre looking at
                displayBoard();
                coords = readAddress();
                row = coords.first; col = coords.second;
                board.printHints(row, col);
            }
            else if (choice == 3) {
                // reset everything back to how it was at the start
                if (confirm("Reset the puzzle? All your progress will be lost.")) {
                    board.resetPuzzle();
                    cout << "Puzzle reset!" << endl;
                    displayBoard();
                    // ask them to pick a cell again since we wiped the board
                    coords = readAddress();
                    row = coords.first; col = coords.second;
                    board.printHints(row, col);
                } else {
                    cout << "Reset cancelled." << endl;
                }
            }
            else if (choice == 4) {
                if (!confirm("Are you sure you want to exit?")) choice = 0;
                else cout << "Goodbye!" << endl;
            }
            else {
                cout << "Invalid choice! Please enter 1-4.\n";
            }
        } while (choice != 4);
    }
};

int main() {
    GameManager game;
    game.run();
    return 0;
}
