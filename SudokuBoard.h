#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Cell.h"
using namespace std;

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
        int sr = (row / 3) * 3, sc = (col / 3) * 3; // top-left corner of the box
        vector<int> used, missing;
        for (int r = sr; r < sr + 3; r++)
            for (int c = sc; c < sc + 3; c++)
                if (grid[r][c].getValue()) used.push_back(grid[r][c].getValue());
        for (int n = 1; n <= 9; n++) {
            bool found = false;
            for (int u : used) if (u == n) found = true;
            if (!found) missing.push_back(n);
        }
        for (int r = sr; r < sr + 3; r++)
            for (int c = sc; c < sc + 3; c++)
                if (!grid[r][c].getValue()) grid[r][c].setBoxHints(missing);
    }

    // when a cell gets updated, we only redo the hints for the
    // row, column, and box that changed — not the whole board
    void reactiveUpdate(int row, int col) {
        recalculateRowHints(row);
        recalculateColumnHints(col);
        recalculateBoxHints(row, col);

        // now we update the consolidated hints for all affected empty cells
        for (int c = 0; c < 9; c++)
            if (!grid[row][c].getValue()) grid[row][c].refreshConsolidatedHints();
        for (int r = 0; r < 9; r++)
            if (!grid[r][col].getValue()) grid[r][col].refreshConsolidatedHints();
        int sr = (row / 3) * 3, sc = (col / 3) * 3;
        for (int r = sr; r < sr + 3; r++)
            for (int c = sc; c < sc + 3; c++)
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
    pair<int, int> translateAddress(string address) {
        if (address.length() < 2) return { -1, -1 };
        char a = toupper(address[0]), b = toupper(address[1]);
        int row, col;
        if (isalpha(a) && isdigit(b)) {
            row = a - 'A';
            col = b - '1';
        }
        else if (isdigit(a) && isalpha(b)) {
            row = b - 'A';
            col = a - '1';
        }
        else return { -1, -1 };
        if (row < 0 || row > 8 || col < 0 || col > 8) return { -1, -1 };
        return { row, col };
    }

    // checks if the answer the user gave is in the consolidated hints
    bool validateAnswer(int row, int col, int answer) {
        vector<int> hints = grid[row][col].getConsolidatedHints();
        if (!hints.empty()) {
            for (int h : hints)
                if (h == answer) return true;
            return false;
        }
        return !isDuplicateInRowColBox(row, col, answer);
    }

    // puts the answer in the cell (only if its not a fixed cell)
    void assignValue(int row, int col, int answer) {
        if (!grid[row][col].getIsFixed()) {
            grid[row][col].setValue(answer);
            reactiveUpdate(row, col);
        }
    }

    // checks if the number already exists somewhere it shouldnt be
    bool isDuplicateInRowColBox(int row, int col, int answer) {
        for (int c = 0; c < 9; c++)
            if (c != col && grid[row][c].getValue() == answer) return true;
        for (int r = 0; r < 9; r++)
            if (r != row && grid[r][col].getValue() == answer) return true;
        int sr = (row / 3) * 3, sc = (col / 3) * 3;
        for (int r = sr; r < sr + 3; r++)
            for (int c = sc; c < sc + 3; c++)
                if (!(r == row && c == col) && grid[r][c].getValue() == answer) return true;
        return false;
    }

    // resets the board back to how it was at the start
    void resetPuzzle() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = originalGrid[r][c];
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                reactiveUpdate(r, c);
    }

    // checks if every cell on the board has a value
    bool isBoardComplete() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (grid[r][c].getValue() == 0) return false;
        return true;
    }

    // prints out all 4 hint arrays for whatever cell you pick
    void printHints(int row, int col) {
        cout << "\nHints for cell " << (char)('A' + row) << (col + 1) << ":" << endl;
        cout << "  Row Hints        : ";
        for (int h : grid[row][col].getRowHints()) cout << h << " ";
        cout << "\n  Column Hints     : ";
        for (int h : grid[row][col].getColHints()) cout << h << " ";
        cout << "\n  Box Hints        : ";
        for (int h : grid[row][col].getBoxHints()) cout << h << " ";
        cout << "\n  Consolidated Hints: ";
        vector<int> ch = grid[row][col].getConsolidatedHints();
        if (ch.empty()) cout << "(none)";
        else for (int h : ch) cout << h << " ";
        cout << endl;
    }

    int getCellValue(int r, int c)    { return grid[r][c].getValue(); }
    bool getCellIsFixed(int r, int c) { return grid[r][c].getIsFixed(); }
};
