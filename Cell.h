#pragma once
#include <vector>
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
