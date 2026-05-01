#include <iostream>
#include <vector>
#include <string>
using namespace std;

// This class handles a single cell in the sudoku grid.
// Each cell knows its value, whether it's locked (fixed),
// and keeps track of 4 hint arrays to help figure out
// what numbers are still possible for that cell.
class Cell {
private:
    int value;        // the number in this cell (0 means empty)
    bool isFixed;     // true if this cell came from the original puzzle

    // these store what numbers are still "missing" from the
    // cell's row, column, and 3x3 box respectively
    vector<int> rowHints, colHints, boxHints;

    // the final list — only numbers that appear in ALL THREE above
    vector<int> consolidatedHints;

public:
    // start everything empty/false by default
    Cell() : value(0), isFixed(false) {}

    // basic getters and setters
    int getValue()          { return value; }
    bool getIsFixed()       { return isFixed; }
    void setValue(int v)    { value = v; }
    void setIsFixed(bool f) { isFixed = f; }

    // these get called by SudokuBoard whenever hints need updating
    void setRowHints(vector<int> h) { rowHints = h; }
    void setColHints(vector<int> h) { colHints = h; }
    void setBoxHints(vector<int> h) { boxHints = h; }

    // finds the intersection of row, col, and box hints.
    // a number only makes it to consolidatedHints if it's
    // present in all three arrays — that's the real "safe" hint
    void refreshConsolidatedHints() {
        consolidatedHints.clear();
        for (int num : rowHints) {
            bool inCol = false, inBox = false;
            for (int c : colHints) if (c == num) inCol = true;
            for (int b : boxHints) if (b == num) inBox = true;
            if (inCol && inBox) consolidatedHints.push_back(num);
        }
    }

    // getters for all 4 hint arrays
    vector<int> getRowHints()          { return rowHints; }
    vector<int> getColHints()          { return colHints; }
    vector<int> getBoxHints()          { return boxHints; }
    vector<int> getConsolidatedHints() { return consolidatedHints; }
};

// This is the main brain of the puzzle. It holds the 9x9 grid
// of Cell objects and handles all the logic — loading, updating
// hints, validating answers, resetting, etc.
class SudokuBoard {
private:
    Cell grid[9][9];         // the board we're actively playing on
    Cell originalGrid[9][9]; // backup copy so we can reset anytime

    // scans the given row to find which numbers are already used,
    // then sets the rowHints for all empty cells in that row
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

    // same idea but for a column
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

    // figures out which 3x3 box the cell belongs to,
    // then updates box hints for all empty cells in that box
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

    // this is the "reactive engine" — when a cell changes,
    // we only update the row, column, and box that were affected.
    // no need to recalculate the whole board every time!
    void reactiveUpdate(int row, int col) {
        recalculateRowHints(row);
        recalculateColumnHints(col);
        recalculateBoxHints(row, col);

        // now refresh consolidated hints for all affected empty cells
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
    // loads the puzzle into the grid, marks fixed cells,
    // saves a backup, then triggers a full hint calculation
    void loadPuzzle(int puzzle[9][9]) {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++) {
                grid[r][c].setValue(puzzle[r][c]);
                grid[r][c].setIsFixed(puzzle[r][c] != 0);
                originalGrid[r][c] = grid[r][c]; // save to backup
            }
        // do a full board update so all hints are ready from the start
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                reactiveUpdate(r, c);
    }

    // converts a user address like "D4" or "4D" into row/col indices
    // handles both formats so the user doesn't get a crash
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

    // checks if the answer the user gave is actually in the
    // consolidated hints — if not, it's wrong
    bool validateAnswer(int row, int col, int answer) {
        for (int h : grid[row][col].getConsolidatedHints())
            if (h == answer) return true;
        return false;
    }

    // places the answer in the cell and immediately triggers
    // reactiveUpdate so neighboring cells get updated hints
    void assignValue(int row, int col, int answer) {
        if (!grid[row][col].getIsFixed()) {
            grid[row][col].setValue(answer);
            reactiveUpdate(row, col); // only updates what's affected
        }
    }

    // restores the board from the backup and recalculates everything
    void resetPuzzle() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = originalGrid[r][c];
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                reactiveUpdate(r, c);
    }

    // prints all 4 hint arrays for a specific cell
    // this gets called from GameManager's printHints method
    void printHints(int row, int col) {
        cout << "Hints for cell " << (char)('A'+row) << (col+1) << ":" << endl;
        cout << "Column Hints: ";
        for (int h : grid[row][col].getColHints()) cout << h << " ";
        cout << "\nRow Hints: ";
        for (int h : grid[row][col].getRowHints()) cout << h << " ";
        cout << "\nBox Hints: ";
        for (int h : grid[row][col].getBoxHints()) cout << h << " ";
        cout << "\nConsolidated Hints: ";
        for (int h : grid[row][col].getConsolidatedHints()) cout << h << " ";
        cout << endl;
    }

    int getCellValue(int r, int c) { return grid[r][c].getValue(); }
};

// This handles everything the user sees and interacts with.
// It talks to SudokuBoard to get things done, but keeps the
class GameManager {
private:
    SudokuBoard board; // our one and only board instance

    // simple yes/no prompt used before important actions
    bool confirm(string message) {
        string r;
        cout << message << " (y/n): ";
        cin >> r;
        return (r == "y" || r == "Y");
    }

    // reads one row from the user and validates it on the spot.
    // won't move on until the row has valid numbers with no duplicates
    void readRow(int puzzle[9][9], int r) {
        bool valid = false;
        while (!valid) {
            cout << "Row " << (char)('A'+r) << ": ";
            for (int c = 0; c < 9; c++) cin >> puzzle[r][c];

            bool ok = true;
            // check that all values are between 0-9
            for (int c = 0; c < 9 && ok; c++)
                if (puzzle[r][c] < 0 || puzzle[r][c] > 9) {
                    cout << "Invalid number! Only 0-9 allowed. Re-enter.\n";
                    ok = false;
                }
            if (!ok) continue;

            // check for duplicates within the same row
            for (int c = 0; c < 9 && ok; c++) {
                if (!puzzle[r][c]) continue;
                for (int k = c+1; k < 9 && ok; k++)
                    if (puzzle[r][c] == puzzle[r][k]) {
                        cout << "Duplicate " << puzzle[r][c] << " in row "
                             << (char)('A'+r) << "! Re-enter.\n";
                        ok = false;
                    }
            }
            valid = ok;
        }
    }

    // after all rows are entered, check the whole board for
    // column and box duplicates. if found, ask the user to fix them
    bool validateBoard(int puzzle[9][9]) {
        // check each column for duplicates
        for (int c = 0; c < 9; c++) {
            for (int r1 = 0; r1 < 9; r1++) {
                if (!puzzle[r1][c]) continue;
                for (int r2 = r1+1; r2 < 9; r2++) {
                    if (puzzle[r1][c] == puzzle[r2][c]) {
                        cout << "Duplicate " << puzzle[r1][c]
                             << " in column " << (c+1) << "! Re-enter affected rows.\n";
                        readRow(puzzle, r1);
                        readRow(puzzle, r2);
                        return false; // restart validation from the top
                    }
                }
            }
        }
        // check each 3x3 box for duplicates
        for (int br = 0; br < 3; br++) {
            for (int bc = 0; bc < 3; bc++) {
                vector<pair<int,int>> cells;
                for (int r = br*3; r < br*3+3; r++)
                    for (int c = bc*3; c < bc*3+3; c++)
                        if (puzzle[r][c]) cells.push_back({r, c});
                for (int i = 0; i < (int)cells.size(); i++)
                    for (int j = i+1; j < (int)cells.size(); j++)
                        if (puzzle[cells[i].first][cells[i].second] ==
                            puzzle[cells[j].first][cells[j].second]) {
                            cout << "Duplicate " << puzzle[cells[i].first][cells[i].second]
                                 << " in box (" << (br+1) << "," << (bc+1) << ")! Re-enter affected rows.\n";
                            readRow(puzzle, cells[i].first);
                            readRow(puzzle, cells[j].first);
                            return false;
                        }
            }
        }
        return true; // all good, no conflicts found
    }

public:
    // draws the current state of the board to the console
    void displayBoard() {
        cout << "\n     1 2 3   4 5 6   7 8 9" << endl;
        cout << "   +-------+-------+-------+" << endl;
        for (int r = 0; r < 9; r++) {
            cout << " " << (char)('A'+r) << " |";
            for (int c = 0; c < 9; c++) {
                int val = board.getCellValue(r, c);
                cout << (val ? " " + to_string(val) : " ."); // dot for empty
                if (c == 2 || c == 5 || c == 8) cout << " |";
            }
            cout << " " << (char)('A'+r) << endl;
            if (r == 2 || r == 5)
                cout << "   +-------+-------+-------+" << endl;
        }
        cout << "   +-------+-------+-------+" << endl;
        cout << "     1 2 3   4 5 6   7 8 9\n" << endl;
    }

    // entry point — asks the user to input the puzzle,
    // validates it, loads it, then starts the menu
    void run() {
        int puzzle[9][9];
        cout << "Enter the Sudoku puzzle row by row." << endl;
        cout << "Use 0 for empty cells, separate numbers with spaces.\n" << endl;

        for (int r = 0; r < 9; r++) readRow(puzzle, r);
        while (!validateBoard(puzzle)); // keep re-checking until no conflicts

        board.loadPuzzle(puzzle);
        displayBoard();
        menu();
    }

    // the main game loop — shows options and handles user choices
    void menu() {
        string input;
        int choice;

        // ask which cell to start with and show its hints
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

        auto coords = readAddress();
        int row = coords.first, col = coords.second;
        board.printHints(row, col);

        do {
            cout << "\n--- MENU ---\n"
                 << "1. Provide an answer for this cell\n"
                 << "2. Check another cell\n"
                 << "3. Reset the puzzle\n"
                 << "4. Exit\n"
                 << "Choice: ";

            // fix: if cin fails (e.g. user types "g1"), clear and skip
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input! Please enter 1-4.\n";
                choice = 0;
                continue;
            }

            if (choice == 1) {
                int answer;
                cout << "Enter your answer: ";
                if (!(cin >> answer) || answer < 1 || answer > 9) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input! Enter a number from 1-9.\n";
                    continue;
                }
                if (board.validateAnswer(row, col, answer)) {
                    // double-check before placing the answer
                    if (confirm("Are you sure you want to place " + to_string(answer) +
                                " in cell " + (char)('A'+row) + to_string(col+1) + "?")) {
                        board.assignValue(row, col, answer);
                        cout << "Answer accepted!" << endl;
                        displayBoard();
                    } else cout << "Action cancelled." << endl;
                } else cout << "Invalid answer! Not in consolidated hints." << endl;
            }
            else if (choice == 2) {
                if (confirm("Do you want to check another cell?")) {
                    coords = readAddress();
                    row = coords.first; col = coords.second;
                    board.printHints(row, col);
                } else cout << "Action cancelled." << endl;
            }
            else if (choice == 3) {
                if (confirm("Do you really want to reset? All progress will be lost")) {
                    board.resetPuzzle();
                    cout << "Puzzle reset!" << endl;
                    displayBoard();
                } else cout << "Reset cancelled." << endl;
            }
            else if (choice == 4) {
                // confirm before quitting
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
