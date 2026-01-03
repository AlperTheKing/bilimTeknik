#include <bits/stdc++.h>
using namespace std;

// Size of the puzzle
static const int N = 6;

// -------------------- Helper functions -------------------- //

// Calculate the sum of “visible” skyscrapers (from left to right).
int sumVisibleLeft(const vector<int> &row) {
    int maxH = 0, total = 0;
    for (int h : row) {
        if (h > maxH) {
            maxH = h;
            total += h;
        }
    }
    return total;
}

// Calculate the sum of “visible” skyscrapers (from right to left).
int sumVisibleRight(const vector<int> &row) {
    int maxH = 0, total = 0;
    for (int i = N-1; i >= 0; i--) {
        if (row[i] > maxH) {
            maxH = row[i];
            total += row[i];
        }
    }
    return total;
}

// Same idea for columns, top->bottom
int sumVisibleTop(const vector<vector<int>> &grid, int col) {
    int maxH = 0, total = 0;
    for (int row = 0; row < N; row++) {
        int h = grid[row][col];
        if (h > maxH) {
            maxH = h;
            total += h;
        }
    }
    return total;
}

// And bottom->top
int sumVisibleBottom(const vector<vector<int>> &grid, int col) {
    int maxH = 0, total = 0;
    for (int row = N-1; row >= 0; row--) {
        int h = grid[row][col];
        if (h > maxH) {
            maxH = h;
            total += h;
        }
    }
    return total;
}

// -------------------- Main solver -------------------- //

// Outside clues for each row (left->right, right->left) and each column (top->bottom, bottom->top).
// REPLACE these with your actual puzzle clues (or -1 if no clue given).
int leftSum [N]   = {-1, -1, -1, 8, -1, -1}; // (-1 means no clue)
int rightSum[N]   = {-1, -1, 9, -1, -1, -1};
int topSum   [N]  = {-1, 15, 7, 13, -1, 16};
int bottomSum[N]  = {19, -1, 14, -1, -1, 10};

// Check if placing row `r` with a given permutation is compatible so far
// (unique in columns, and if we have row clues, check them).
bool isValidRow(const vector<vector<int>> &grid, int r, const vector<int> &candidate) {
    // Check columns uniqueness with already placed rows
    for (int c = 0; c < N; c++) {
        // candidate[c] is the proposed number in row r, column c
        for (int rr = 0; rr < r; rr++) {
            if (grid[rr][c] == candidate[c]) {
                return false; // same column used the same number
            }
        }
    }
    // If there's a left->right clue, check
    if (leftSum[r] != -1) {
        if (sumVisibleLeft(candidate) != leftSum[r]) {
            return false;
        }
    }
    // If there's a right->left clue, check
    if (rightSum[r] != -1) {
        if (sumVisibleRight(candidate) != rightSum[r]) {
            return false;
        }
    }
    return true;
}

// After fully filling the grid, we must check column sums if they are given.
bool checkAllColumns(const vector<vector<int>> &grid) {
    for (int c = 0; c < N; c++) {
        if (topSum[c] != -1) {
            if (sumVisibleTop(grid, c) != topSum[c]) {
                return false;
            }
        }
        if (bottomSum[c] != -1) {
            if (sumVisibleBottom(grid, c) != bottomSum[c]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Precompute all permutations of {1..6}:
    vector<int> basePerm(N);
    iota(basePerm.begin(), basePerm.end(), 1);

    vector<vector<int>> allPerms; 
    do {
        allPerms.push_back(basePerm);
    } while (next_permutation(basePerm.begin(), basePerm.end()));
    
    // We will store the final solution here
    vector<vector<int>> solution(N, vector<int>(N, 0));
    bool solved = false;

    // Backtracking function (lambda) that tries to assign row r
    function<void(int)> backtrack = [&](int r) {
        if (r == N) {
            // All rows assigned, check column sums:
            if (checkAllColumns(solution)) {
                solved = true;
            }
            return;
        }

        // Try each permutation of {1..6} for row r
        for (auto &perm : allPerms) {
            if (!isValidRow(solution, r, perm)) 
                continue;
            // Place it
            solution[r] = perm;
            // Go next row
            backtrack(r + 1);
            if (solved) return; // stop if solution found
        }
    };

    backtrack(0);

    if (solved) {
        cout << "Solution:\n";
        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                cout << solution[r][c] << " ";
            }
            cout << "\n";
        }
    } else {
        cout << "No solution found (check your clues).\n";
    }

    return 0;
}