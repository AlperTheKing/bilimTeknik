#include <bits/stdc++.h>
using namespace std;

constexpr int N = 6;

// Clues for each side; 0 means “no clue”
int topClue   [N] = {3, 0, 0, 3, 3, 0};
int bottomClue[N] = {0, 3, 3, 3, 0, 3};
int leftClue  [N] = {0, 0, 4, 0, 0, 0};
int rightClue [N] = {3, 0, 2, 0, 4, 0};

int grid[N][N];
bool usedRow[N][N+1], usedCol[N][N+1], usedD1[N+1], usedD2[N+1];

// count how many “buildings” are visible from the front of a 1..N array
int visibleCount(const vector<int>& a) {
    int mx = 0, vis = 0;
    for (int x : a) {
        if (x > mx) {
            mx = x;
            ++vis;
        }
    }
    return vis;
}

// extract a column into a vector
vector<int> getCol(int c) {
    vector<int> v(N);
    for (int r = 0; r < N; ++r) v[r] = grid[r][c];
    return v;
}

bool dfs(int r, int c) {
    if (r == N) {
        // solved: print
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cout << grid[i][j] << (j+1<N?' ':'\n');
            }
        }
        return true;
    }
    int nr = r, nc = c+1;
    if (nc == N) {
        nr = r+1;
        nc = 0;
    }

    bool onD1 = (r == c);
    bool onD2 = (r + c == N-1);

    for (int v = 1; v <= N; ++v) {
        if (usedRow[r][v] || usedCol[c][v]) continue;
        if (onD1 && usedD1[v]) continue;
        if (onD2 && usedD2[v]) continue;

        // place
        grid[r][c] = v;
        usedRow[r][v] = usedCol[c][v] = true;
        if (onD1) usedD1[v] = true;
        if (onD2) usedD2[v] = true;

        bool ok = true;

        // if end of row, check left/right clues
        if (c == N-1) {
            vector<int> rowv(grid[r], grid[r] + N);
            int visL = visibleCount(rowv);
            int visR = visibleCount(vector<int>(rowv.rbegin(), rowv.rend()));
            if (leftClue[r]  && visL != leftClue[r])  ok = false;
            if (rightClue[r] && visR != rightClue[r]) ok = false;
        }

        // if end of column, check top/bottom clues
        if (ok && r == N-1) {
            auto colv = getCol(c);
            int visT = visibleCount(colv);
            int visB = visibleCount(vector<int>(colv.rbegin(), colv.rend()));
            if (topClue[c]    && visT != topClue[c])    ok = false;
            if (bottomClue[c] && visB != bottomClue[c]) ok = false;
        }

        if (ok && dfs(nr, nc)) return true;

        // undo
        usedRow[r][v] = usedCol[c][v] = false;
        if (onD1) usedD1[v] = false;
        if (onD2) usedD2[v] = false;
    }

    return false;
}

int main() {
    // initialize
    memset(usedRow,  false, sizeof usedRow);
    memset(usedCol,  false, sizeof usedCol);
    memset(usedD1,   false, sizeof usedD1);
    memset(usedD2,   false, sizeof usedD2);

    if (!dfs(0,0)) {
        cout << "No solution found\n";
    }
    return 0;
}