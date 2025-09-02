#include <bits/stdc++.h>
using namespace std;

static const int N = 6;                     // 6x6 Sudoku
static const int ALL = (1<<N) - 1;          // bitmask for {1..6}
int grid[N][N];                             // 0 = empty, 1..6 = value
int regionId[N][N];                         // region indices 0..5

// Knight move offsets
const int KDIR[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};

// Helpers
inline bool inside(int r,int c){ return r>=0 && r<N && c>=0 && c<N; }
inline int bit(int d){ return 1<<(d-1); }   // d in 1..6

struct Solver {
    int rowMask[N]{}, colMask[N]{}, boxMask[6]{};
    int solCount = 0;
    int bestSol[N][N]{};

    // Initialize masks from given clues
    bool initMasks() {
        memset(rowMask, 0, sizeof(rowMask));
        memset(colMask, 0, sizeof(colMask));
        memset(boxMask, 0, sizeof(boxMask));
        for(int r=0;r<N;r++){
            for(int c=0;c<N;c++){
                int v = grid[r][c];
                if(v==0) continue;
                int b = regionId[r][c];
                // Row/column/region conflict
                if( (rowMask[r] & bit(v)) || (colMask[c] & bit(v)) || (boxMask[b] & bit(v)) )
                    return false;
                // Anti-knight conflict
                for(auto &d: KDIR){
                    int nr=r+d[0], nc=c+d[1];
                    if(inside(nr,nc) && grid[nr][nc]==v) return false;
                }
                rowMask[r] |= bit(v);
                colMask[c] |= bit(v);
                boxMask[b] |= bit(v);
            }
        }
        return true;
    }

    // Candidate mask for a cell (including anti-knight)
    int candidates(int r,int c) {
        int b = regionId[r][c];
        int m = ~(rowMask[r] | colMask[c] | boxMask[b]) & ALL;
        // Filter out candidates blocked by same digit on knight moves
        int ok = 0;
        for(int d=1; d<=N; ++d){
            if(!(m & bit(d))) continue;
            bool clash=false;
            for(auto &k: KDIR){
                int nr=r+k[0], nc=c+k[1];
                if(inside(nr,nc) && grid[nr][nc]==d){ clash=true; break; }
            }
            if(!clash) ok |= bit(d);
        }
        return ok;
    }

    // Pick the most constrained empty cell (MRV)
    // Return: -1 = contradiction, 0 = solved (no empty cells), 1 = continue (a cell was selected)
    int pick_cell(int &br, int &bc, int &candMask) {
        int bestCnt = 99;
        br = bc = -1; candMask = 0;
        for(int r=0;r<N;r++){
            for(int c=0;c<N;c++){
                if(grid[r][c]!=0) continue;
                int cm = candidates(r,c);
                int cnt = __builtin_popcount((unsigned)cm);
                if(cnt==0) return -1; // contradiction
                if(cnt < bestCnt){
                    bestCnt = cnt; br=r; bc=c; candMask=cm;
                }
            }
        }
        if(br==-1) return 0; // no empty cells -> solved
        return 1;            // a cell was selected
    }

    void dfs(){
        if(solCount>1) return; // early exit for uniqueness check
        int r,c,cm;
        int state = pick_cell(r,c,cm);
        if(state == -1) return; // contradiction
        if(state == 0){         // solved
            solCount++;
            if(solCount==1){
                for(int i=0;i<N;i++)
                    for(int j=0;j<N;j++)
                        bestSol[i][j]=grid[i][j];
            }
            return;
        }
        int b = regionId[r][c];
        // Try candidates (lowest bit first)
        for(int d=1; d<=N; ++d){
            if(!(cm & bit(d))) continue;
            // place
            grid[r][c]=d;
            rowMask[r] |= bit(d);
            colMask[c] |= bit(d);
            boxMask[b] |= bit(d);
            bool ok=true;
            // Forward anti-knight check: no same digit on knight moves
            for(auto &k: KDIR){
                int nr=r+k[0], nc=c+k[1];
                if(inside(nr,nc) && grid[nr][nc]==d){ ok=false; break; }
            }
            if(ok) dfs();
            // backtrack
            rowMask[r] &= ~bit(d);
            colMask[c] &= ~bit(d);
            boxMask[b] &= ~bit(d);
            grid[r][c]=0;
            if(solCount>1) return;
        }
    }
};

static void printBoard(const int a[N][N]){
    for(int r=0;r<N;r++){
        for(int c=0;c<N;c++){
            cout << a[r][c] << (c==N-1?'\n':' ');
        }
    }
}

int main(){
    // --- GIVEN CLUES (from the puzzle) ---
    int given[N][N] = {
        {0,0,2,0,0,0},   // r1
        {0,0,0,0,0,0},   // r2
        {0,0,0,0,0,0},   // r3
        {0,6,0,0,0,0},   // r4
        {0,0,0,3,0,0},   // r5
        {1,5,0,0,0,0}    // r6
    };

    // --- REGION MAP ---
    // Irregular (jigsaw) regions matching the screenshot; each region id 0..5 must have exactly 6 cells.
    int regions[N][N] = {
        {0,0,0,0,0,3},   // r1
        {1,1,0,2,3,3},   // r2
        {1,1,2,2,3,3},   // r3
        {4,1,2,2,2,3},   // r4
        {4,1,5,5,5,5},   // r5
        {4,4,4,4,5,5}    // r6
    };

    // If you use a different puzzle layout, just update the `regions` matrix so that ids 0..5 each cover 6 cells.

    // Copy into working arrays
    for(int r=0;r<N;r++){
        for(int c=0;c<N;c++){
            grid[r][c] = given[r][c];
            regionId[r][c] = regions[r][c];
        }
    }

    Solver S;
    if(!S.initMasks()){
        cerr << "Contradictory givens: initial rule violation.\n";
        return 0;
    }
    S.dfs();

    if(S.solCount==0){
        cout << "No solution.\n";
    }else{
        cout << "Found solution:\n";
        printBoard(S.bestSol);
        if(S.solCount==1) cout << "(Unique solution)\n";
        else              cout << "(Multiple solutions)\n";
    }
    return 0;
}