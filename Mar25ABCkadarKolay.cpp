#include <bits/stdc++.h>
using namespace std;

// We have a 6×6 puzzle:
static const int N = 6;

// The letters we must use exactly once in each row/column:
static const vector<char> ABCD = {'A','B','C','D'};

// Outside “first seen” clues for each row (left/right) and each column (top/bottom).
// Fill them in with '.' where there is no clue.
char leftRow[N]   = {'.', '.', 'B', 'B', '.', '.'};
char rightRow[N]  = {'D', 'C', '.', 'D', '.', 'C'};
char topCol[N]    = {'.', '.', '.', 'C', '.', '.'};
char bottomCol[N] = {'.', 'B', 'B', '.', '.', '.'};

// Our 6×6 grid.  Each cell is either '.' (blank) or A/B/C/D.
char grid[N][N];

// rowUsed[r][i] = true means row r already has the letter ABCD[i]
bool rowUsed[N][4];
// colUsed[c][i] = true means column c already has the letter ABCD[i]
bool colUsed[N][4];

// For enforcing “exactly 4 letters per row”:
int lettersInRow[N];

/**
 * Check the "view from left" clue for row r:
 *  If leftRow[r] != '.', the first letter in row r (from left) must match it.
 */
bool checkLeftClue(int r) {
    if (leftRow[r] == '.') return true; 
    for (int c = 0; c < N; c++) {
        if (grid[r][c] != '.') {
            return (grid[r][c] == leftRow[r]);
        }
    }
    // If the row is entirely blank, it fails the clue.
    return false;
}

/**
 * Check the "view from right" clue for row r.
 */
bool checkRightClue(int r) {
    if (rightRow[r] == '.') return true;
    for (int c = N - 1; c >= 0; c--) {
        if (grid[r][c] != '.') {
            return (grid[r][c] == rightRow[r]);
        }
    }
    return false;
}

/**
 * Check the "view from top" clue for column c.
 */
bool checkTopClue(int c) {
    if (topCol[c] == '.') return true;
    for (int r = 0; r < N; r++) {
        if (grid[r][c] != '.') {
            return (grid[r][c] == topCol[c]);
        }
    }
    return false;
}

/**
 * Check the "view from bottom" clue for column c.
 */
bool checkBottomClue(int c) {
    if (bottomCol[c] == '.') return true;
    for (int r = N - 1; r >= 0; r--) {
        if (grid[r][c] != '.') {
            return (grid[r][c] == bottomCol[c]);
        }
    }
    return false;
}

/**
 * To ensure each column also contains A/B/C/D exactly once, we need to verify
 * that each letter actually appears once in each column.  We do this *after*
 * we fill the grid (or whenever the grid is complete).
 */
bool checkAllColumnsHaveABCD() {
    for (int c = 0; c < N; c++) {
        // Track whether we found A,B,C,D in this column:
        vector<bool> found(4,false);
        for (int r = 0; r < N; r++) {
            char ch = grid[r][c];
            if (ch != '.') {
                int idx = -1;
                for (int i = 0; i < 4; i++) {
                    if (ABCD[i] == ch) {
                        idx = i;
                        break;
                    }
                }
                if (idx == -1) return false; // shouldn't happen in this puzzle
                found[idx] = true;
            }
        }
        // Each letter must appear exactly once, so all found[] must be true:
        for (int i = 0; i < 4; i++) {
            if (!found[i]) return false;
        }
    }
    return true;
}

/**
 * Check all outside clues at once:
 */
bool checkAllClues() {
    // Check left/right clues row by row
    for (int r = 0; r < N; r++) {
        if (!checkLeftClue(r))  return false;
        if (!checkRightClue(r)) return false;
    }
    // Check top/bottom clues column by column
    for (int c = 0; c < N; c++) {
        if (!checkTopClue(c))    return false;
        if (!checkBottomClue(c)) return false;
    }
    // Also ensure each column truly has A,B,C,D once:
    if (!checkAllColumnsHaveABCD()) return false;

    return true;
}

/**
 * Backtracking function to fill the grid row by row.
 * r, c = current cell to fill
 */
bool solvePuzzle(int r, int c) {
    // If we've gone past the bottom row, check final constraints:
    if (r == N) {
        // Check all edge/clue constraints:
        return checkAllClues();
    }

    // If we've gone past the last column in row r, move to next row
    if (c == N) {
        // We must have exactly 4 letters in row r:
        if (lettersInRow[r] != 4) return false;

        // Quick check on row's left/right clues (optional, but can prune early):
        if (!checkLeftClue(r) || !checkRightClue(r)) {
            return false;
        }
        // Move to row r+1
        return solvePuzzle(r+1, 0);
    }

    // If row r already has 4 letters, the rest must be blank
    if (lettersInRow[r] == 4) {
        grid[r][c] = '.';
        return solvePuzzle(r, c+1);
    }

    // Try placing '.' (blank):
    grid[r][c] = '.';
    if (solvePuzzle(r, c+1)) {
        return true;
    }

    // Otherwise, try each letter A,B,C,D (if not used yet in row/column):
    for (int i = 0; i < 4; i++) {
        char L = ABCD[i];
        if (rowUsed[r][i]) continue;  // row r already has letter L
        if (colUsed[c][i]) continue;  // column c already has letter L

        // Place L here
        grid[r][c] = L;
        rowUsed[r][i] = true;
        colUsed[c][i] = true;
        lettersInRow[r]++;

        if (solvePuzzle(r, c+1)) {
            return true;
        }

        // Backtrack
        grid[r][c] = '.';
        rowUsed[r][i] = false;
        colUsed[c][i] = false;
        lettersInRow[r]--;
    }

    // No success placing anything here
    return false;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Initialize everything
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < N; c++) {
            grid[r][c] = '.';
        }
        for (int i = 0; i < 4; i++) {
            rowUsed[r][i] = false;
        }
        lettersInRow[r] = 0;
    }
    for (int c = 0; c < N; c++) {
        for (int i = 0; i < 4; i++) {
            colUsed[c][i] = false;
        }
    }

    // Attempt to solve
    bool ok = solvePuzzle(0, 0);
    if (!ok) {
        cout << "No solution found.\n";
    } else {
        cout << "Solution:\n";
        for (int r = 0; r < N; r++) {
            for (int c = 0; c < N; c++) {
                cout << grid[r][c] << ' ';
            }
            cout << "\n";
        }
    }
    return 0;
}