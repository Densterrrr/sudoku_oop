#pragma once
#include <iostream>
#include <string>
#include "SudokuBoard.h"
using namespace std;

// this class handles everything the user sees and interacts with
// it basically separates the display/input stuff from the board logic

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
    void displayBoard() {
        cout << "\n     1 2 3   4 5 6   7 8 9" << endl;
        cout << "   +-------+-------+-------+" << endl;
        for (int r = 0; r < 9; r++) {
            cout << " " << (char)('A' + r) << " |";
            for (int c = 0; c < 9; c++) {
                int val = board.getCellValue(r, c);
                cout << (val ? " " + to_string(val) : " ."); // dot if empty
                if (c == 2 || c == 5 || c == 8) cout << " |";
            }
            cout << " " << (char)('A' + r) << endl;
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
            {5, 3, 0, 0, 7, 0, 0, 0, 0},
            {6, 0, 0, 1, 9, 5, 0, 0, 0},
            {0, 9, 8, 0, 0, 0, 0, 6, 0},
            {8, 0, 0, 0, 6, 0, 0, 0, 3},
            {4, 0, 0, 8, 0, 3, 0, 0, 1},
            {7, 0, 0, 0, 2, 0, 0, 0, 6},
            {0, 6, 0, 0, 0, 0, 2, 8, 0},
            {0, 0, 0, 4, 1, 9, 0, 0, 5},
            {0, 0, 0, 0, 8, 0, 0, 7, 9}
        };

        board.loadPuzzle(puzzle);
        cout << "Welcome to our Sudoku puzzle!" << endl;
        cout << "Here's the starting board:" << endl;
        displayBoard();
        displayMenu();
    }

    // the main loop — keeps showing the menu until the user exits
    void displayMenu() {
        string input;
        int choice;

        // lambda that keeps asking until we get a valid cell address
        auto readAddress = [&]() {
            pair<int, int> coords = { -1, -1 };
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
            cout << "\n--- MENU ---\n"
                << "1. Provide an answer for this cell\n"
                << "2. Check another cell\n"
                << "3. Reset the puzzle\n"
                << "4. Exit\n"
                << "Choice: ";

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input! Please enter 1-4.\n";
                choice = 0;
                continue;
            }

            if (choice == 1) {
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

                if (answer == 0) {
                    board.assignValue(row, col, 0);
                    cout << "Cell cleared!" << endl;
                    displayBoard();
                    continue;
                }

                if (board.isDuplicateInRowColBox(row, col, answer)) {
                    cout << "Invalid! " << answer
                        << " already exists in the same row, column, or box." << endl;
                }
                else if (!board.validateAnswer(row, col, answer)) {
                    cout << "Invalid! " << answer
                        << " is not in the consolidated hints for this cell." << endl;
                    cout << "Tip: Use option 2 to view the hints for this cell again." << endl;
                }
                else {
                    board.assignValue(row, col, answer);
                    cout << "Answer accepted!" << endl;
                    displayBoard();

                    if (board.isBoardComplete()) {
                        cout << "Congratulations! Puzzle Solved!" << endl;
                        return;
                    }
                }
            }
            else if (choice == 2) {
                displayBoard();
                coords = readAddress();
                row = coords.first; col = coords.second;
                board.printHints(row, col);
            }
            else if (choice == 3) {
                if (confirm("Reset the puzzle? All your progress will be lost.")) {
                    board.resetPuzzle();
                    cout << "Puzzle reset!" << endl;
                    displayBoard();
                    coords = readAddress();
                    row = coords.first; col = coords.second;
                    board.printHints(row, col);
                }
                else {
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
