#include <iostream>
#include <vector>
#include <set>
using namespace std;

static const int SIZE = 6;

bool is_valid(vector<vector<int>> &board,
              const vector<set<pair<int,int>>> &regions,
              const vector<vector<pair<int,int>>> &thermometers,
              int r, int c, int val)
{
    // Check row
    for (int col = 0; col < SIZE; ++col) {
        if (board[r][col] == val) return false;
    }

    // Check column
    for (int row = 0; row < SIZE; ++row) {
        if (board[row][c] == val) return false;
    }

    // Check region
    for (auto &region : regions) {
        if (region.find({r,c}) != region.end()) {
            // region found, check if val already in that region
            for (auto &cell : region) {
                int rr = cell.first;
                int cc = cell.second;
                if (board[rr][cc] == val) return false;
            }
            break;
        }
    }

    // Temporarily place val to check thermometers
    int original = board[r][c];
    board[r][c] = val;

    // Check thermometer constraints
    for (auto &thermo : thermometers) {
        vector<int> thermo_values;
        thermo_values.reserve(thermo.size());
        for (auto &tcell : thermo) {
            thermo_values.push_back(board[tcell.first][tcell.second]);
        }

        // If any zero inside, it's not fully constrained yet, no check needed now
        bool has_zero = false;
        for (auto v : thermo_values) {
            if (v == 0) {
                has_zero = true;
                break;
            }
        }
        if (has_zero) continue;

        // Values must be strictly increasing
        for (int i = 0; i < (int)thermo_values.size() - 1; ++i) {
            if (thermo_values[i] >= thermo_values[i+1]) {
                board[r][c] = original;
                return false;
            }
        }
    }

    // revert
    board[r][c] = original;
    return true;
}

bool backtrack(vector<vector<int>> &board,
               const vector<set<pair<int,int>>> &regions,
               const vector<vector<pair<int,int>>> &thermometers)
{
    // Find the next empty cell
    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {
            if (board[row][col] == 0) {
                // Try values 1 to 6
                for (int val = 1; val <= 6; ++val) {
                    if (is_valid(board, regions, thermometers, row, col, val)) {
                        board[row][col] = val;
                        if (backtrack(board, regions, thermometers)) {
                            return true;
                        }
                        // backtrack
                        board[row][col] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true; // no empty cell found, puzzle solved
}

bool solve_thermometer_sudoku(vector<vector<int>> &board,
                              const vector<set<pair<int,int>>> &regions,
                              const vector<vector<pair<int,int>>> &thermometers)
{
    return backtrack(board, regions, thermometers);
}

int main() {
    // Example board (all zeros, no givens). Replace zeros with known digits if any.
    vector<vector<int>> board = {
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0},
        {0,0,0,0,0,0}
    };

    // Define your regions
    // As given in your code. Adjust if needed.
    vector<set<pair<int,int>>> regions = {
        {{0,0},{0,1},{0,2},{1,0},{1,1},{1,2}},
        {{0,3},{0,4},{0,5},{1,3},{1,4},{1,5}},
        {{2,0},{2,1},{2,2},{3,0},{3,1},{3,2}},
        {{2,3},{2,4},{2,5},{3,3},{3,4},{3,5}},
        {{4,0},{4,1},{4,2},{5,0},{5,1},{5,2}},
        {{4,3},{4,4},{4,5},{5,3},{5,4},{5,5}}
    };

    // Define your thermometers
    vector<vector<pair<int,int>>> thermometers = {
        {{0,2},{1,1},{2,0},{3,1},{2,2}},
        {{5,0},{4,1},{3,2},{2,3},{1,4}},
        {{4,4},{3,3},{2,4},{1,5}},
        };

    // Solve the puzzle
    bool solved = solve_thermometer_sudoku(board, regions, thermometers);

    if (solved) {
        for (auto &row : board) {
            for (auto val : row) {
                cout << val << " ";
            }
            cout << "\n";
        }
    } else {
        cout << "No solution found.\n";
    }

    return 0;
}
