#include <bits/stdc++.h>
using namespace std;

struct Cage{
    int target;
    vector<pair<int,int>> cells;
    vector<vector<int>> combos;
};

int N=6;
vector<Cage> cages;

int pc(int x){ return __builtin_popcount((unsigned)x); }
int lowv(int m){ return __builtin_ctz((unsigned)m)+1; }

void build_combos(Cage& cg){
    int k=cg.cells.size();
    cg.combos.clear();
    if(k==1){
        if(1<=cg.target && cg.target<=6) cg.combos.push_back({cg.target});
        return;
    }
    if(k==2){
        for(int a=1;a<=6;++a) for(int b=1;b<=6;++b){
            if(a+b==cg.target || abs(a-b)==cg.target || a*b==cg.target || (max(a,b)%min(a,b)==0 && max(a,b)/min(a,b)==cg.target))
                cg.combos.push_back({a,b});
        }
        return;
    }
    vector<int> t(k,1);
    function<void(int)> dfs=[&](int i){
        if(i==k){
            int s=0; long long p=1;
            for(int v:t){ s+=v; p*=v; }
            if(s==cg.target || p==cg.target) cg.combos.push_back(t);
            return;
        }
        for(int v=1; v<=6; ++v){ t[i]=v; dfs(i+1); }
    };
    dfs(0);
}

struct State{
    int mask[6][6];
    int rowUsed[6], colUsed[6];
};

bool propagate(State& st){
    bool changed=true;
    while(changed){
        changed=false;
        for(int r=0;r<6;++r) for(int c=0;c<6;++c){
            if(pc(st.mask[r][c])==1) continue;
            int m = st.mask[r][c];
            m &= ~st.rowUsed[r];
            m &= ~st.colUsed[c];
            if(m==0) return false;
            if(m!=st.mask[r][c]){ st.mask[r][c]=m; changed=true; }
        }
        for(auto& cg:cages){
            int k=cg.cells.size();
            vector<int> allow(k,0);
            for(auto &comb:cg.combos){
                bool ok=true;
                for(int i=0;i<k;++i){
                    int r=cg.cells[i].first, c=cg.cells[i].second;
                    if(!(st.mask[r][c] & (1<<(comb[i]-1)))){ ok=false; break; }
                }
                if(!ok) continue;
                for(int i=0;i<k;++i) allow[i] |= 1<<(comb[i]-1);
            }
            for(int i=0;i<k;++i){
                int r=cg.cells[i].first, c=cg.cells[i].second;
                int m = st.mask[r][c] & allow[i];
                if(m==0) return false;
                if(m!=st.mask[r][c]){ st.mask[r][c]=m; changed=true; }
            }
        }
        for(int r=0;r<6;++r){
            for(int c=0;c<6;++c){
                if(pc(st.mask[r][c])==1){
                    int v=lowv(st.mask[r][c]);
                    if(!(st.rowUsed[r]&(1<<(v-1)))){ st.rowUsed[r]|=1<<(v-1); changed=true; }
                    if(!(st.colUsed[c]&(1<<(v-1)))){ st.colUsed[c]|=1<<(v-1); changed=true; }
                }
            }
        }
        for(int r=0;r<6;++r){
            for(int d=1; d<=6; ++d){
                if(st.rowUsed[r]&(1<<(d-1))) continue;
                int cnt=0,pos=-1;
                for(int c=0;c<6;++c) if(st.mask[r][c]&(1<<(d-1))){ cnt++; pos=c; }
                if(cnt==0) return false;
                if(cnt==1){
                    int c=pos; int m=1<<(d-1);
                    if(st.mask[r][c]!=m){ st.mask[r][c]=m; changed=true; }
                }
            }
        }
        for(int c=0;c<6;++c){
            for(int d=1; d<=6; ++d){
                if(st.colUsed[c]&(1<<(d-1))) continue;
                int cnt=0,pos=-1;
                for(int r=0;r<6;++r) if(st.mask[r][c]&(1<<(d-1))){ cnt++; pos=r; }
                if(cnt==0) return false;
                if(cnt==1){
                    int r=pos; int m=1<<(d-1);
                    if(st.mask[r][c]!=m){ st.mask[r][c]=m; changed=true; }
                }
            }
        }
    }
    return true;
}

bool solved(const State& st){
    for(int r=0;r<6;++r) for(int c=0;c<6;++c) if(pc(st.mask[r][c])!=1) return false;
    return true;
}

bool dfs(State& st){
    if(!propagate(st)) return false;
    if(solved(st)) return true;
    int br=-1, bc=-1, best=10;
    for(int r=0;r<6;++r) for(int c=0;c<6;++c){
        int p=pc(st.mask[r][c]);
        if(p>1 && p<best){ best=p; br=r; bc=c; }
    }
    int m=st.mask[br][bc];
    for(int d=1; d<=6; ++d) if(m&(1<<(d-1))){
        State nx=st;
        nx.mask[br][bc]=1<<(d-1);
        nx.rowUsed[br] |= 1<<(d-1);
        nx.colUsed[bc] |= 1<<(d-1);
        if(dfs(nx)){ st=nx; return true; }
    }
    return false;
}

int main(){
    cages = {
        {10, {{0,0},{1,0},{1,1}}},        // 1
        { 2, {{0,1},{0,2}}},              // 2
        {10, {{0,3},{0,4},{0,5}}},        // 3
        {30, {{2,0},{2,1}}},              // 4
        { 3, {{1,2},{2,2}}},              // 5
        {24, {{1,3},{1,4}}},              // 6
        { 2, {{2,3},{2,4}}},              // 7
        {13, {{1,5},{2,5},{3,5}}},        // 8
        { 2, {{3,0},{3,1}}},              // 9
        { 3, {{3,2},{4,2}}},              // 10
        { 7, {{3,3},{4,3},{3,4}}},        // 11
        {10, {{4,0},{5,0}}},              // 12
        { 5, {{4,1},{5,1}}},              // 13
        { 7, {{5,2},{5,3}}},              // 14
        {11, {{4,4},{5,4}}},              // 15
        { 5, {{4,5},{5,5}}}               // 16
    };
    for(auto& cg:cages) build_combos(cg);
    State st;
    for(int r=0;r<6;++r){ st.rowUsed[r]=0; for(int c=0;c<6;++c) st.mask[r][c]=(1<<6)-1; }
    for(int c=0;c<6;++c) st.colUsed[c]=0;
    bool ok=dfs(st);
    if(!ok){ cout<<"No solution\n"; return 0; }
    for(int r=0;r<6;++r){
        for(int c=0;c<6;++c){
            int v=lowv(st.mask[r][c]);
            if(c) cout<<' ';
            cout<<v;
        }
        cout<<"\n";
    }
    return 0;
}