#include <bits/stdc++.h>
using namespace std;

enum Op { ADD, SUB, MUL, DIV };

// A “cage” consists of a target number, an operation, and a list of its cells
struct Cage {
    int target;
    Op op;
    vector<pair<int,int>> cells;
};

static const int N = 6;
vector<Cage> cages;
int grid[N][N];
bool rowUsed[N][N+1], colUsed[N][N+1];

// Check that a (possibly partially‐filled) cage still can reach its target
bool checkCage(const Cage &c, bool finalCheck=false) {
    vector<int> v;
    int sum = 0, prod = 1;
    for (auto &p : c.cells) {
        int x = grid[p.first][p.second];
        if (x == 0) {
            // for ADD/MUL we can do a partial ≤ check; for SUB/DIV wait until filled
            if (c.op==ADD || c.op==MUL) continue;
            else return true;
        }
        v.push_back(x);
        sum  += x;
        prod *= x;
    }
    switch(c.op) {
      case ADD:
        return finalCheck ? (sum == c.target) : (sum <= c.target);
      case MUL:
        return finalCheck ? (prod == c.target) : (prod <= c.target);
      case SUB:
        if (v.size() < 2) return true;
        return abs(v[0] - v[1]) == c.target;
      case DIV:
        if (v.size() < 2) return true;
        {
          int a = max(v[0],v[1]), b = min(v[0],v[1]);
          return b>0 && a%b==0 && a/b==c.target;
        }
    }
    return false;
}

// Backtrack in row‐major order
bool solve(int idx = 0) {
    if (idx == N*N) {
        // final validation
        for (auto &c : cages)
            if (!checkCage(c, true))
                return false;
        return true;
    }
    int r = idx / N, c = idx % N;
    if (grid[r][c] != 0)
        return solve(idx+1);

    for (int d = 1; d <= N; ++d) {
        if (rowUsed[r][d] || colUsed[c][d]) continue;
        grid[r][c] = d;
        rowUsed[r][d] = colUsed[c][d] = true;

        bool ok = true;
        // only re‐check the cage(s) that include (r,c)
        for (auto &cg : cages) {
            for (auto &cell : cg.cells) {
                if (cell.first==r && cell.second==c) {
                    if (!checkCage(cg, false)) ok = false;
                    break;
                }
            }
            if (!ok) break;
        }

        if (ok && solve(idx+1))
            return true;

        // undo
        grid[r][c] = 0;
        rowUsed[r][d] = colUsed[c][d] = false;
    }
    return false;
}

int main(){
    memset(grid,    0, sizeof(grid));
    memset(rowUsed, 0, sizeof(rowUsed));
    memset(colUsed, 0, sizeof(colUsed));

    // --- cages initializer (all coordinates 0‐based) ---
    cages = {
        {10, ADD, {{0,0},{0,1},{0,2}}},           // 10+ in (1,1),(1,2),(1,3)
        { 2, DIV, {{0,3},{1,3}}},                 // 2/  in (1,4),(2,4)
        { 7, ADD, {{0,4},{1,4}}},                 // 7+  in (1,5),(2,5)
        { 5, DIV, {{0,5},{1,5}}},                 // 5/  in (1,6),(2,6)
        { 9, ADD, {{1,0},{2,0}}},                 // 9+  in (2,1),(3,1)
        { 9, ADD, {{1,1},{2,1}}},                 // 9+  in (2,2),(3,2)
        { 6, MUL, {{1,2},{2,2}}},                 // 6×  in (2,3),(3,3)
        {12, MUL, {{2,3},{2,4},{3,3}}},           // 12× in (3,4),(3,5),(4,4)
        {11, ADD, {{2,5},{3,5},{4,5}}},           // 11+ in (3,6),(4,6),(5,6)
        { 6, MUL, {{3,0},{4,0}}},                 // 6×  in (4,1),(5,1)
        {18, ADD, {{3,1},{3,2},{4,1},{4,2},{5,2}}}, // 18+ in (4,2),(4,3),(5,2),(5,3),(6,3)
        { 3, SUB, {{3,4},{4,4}}},                 // 3–  in (4,5),(5,5)
        { 1, SUB, {{4,3},{5,3}}},                 // 1–  in (5,4),(6,4)
        { 2, MUL, {{5,0},{5,1}}},                 // 2×  in (6,1),(6,2)
        {12, MUL, {{5,4},{5,5}}}                  // 12× in (6,5),(6,6)
    };

    if (solve()) {
        cout << "Solution:\n";
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j)
                cout << grid[i][j] << ' ';
            cout << "\n";
        }
    } else {
        cout << "No solution found.\n";
    }
    return 0;
}