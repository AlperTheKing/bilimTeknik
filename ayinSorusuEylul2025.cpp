// ali_baba_bitboard.cpp
// Randomized Greedy with bitboards + 1-step lookahead for the Ali Baba lock.
// Board coords (internal): r=0..7 is TOP->BOTTOM, c=0..7 is LEFT->RIGHT.
// Problem output coords: (Column = c+1, Row = 8 - r), i.e. (1,1) is bottom-left (A).

#include <bits/stdc++.h>
using namespace std;

// Fast 64-bit hash for per-restart seeding
static inline uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

struct Step {
    int r, c;      // 0-based, top-left origin
    int coeff;     // 0..4 at placement time
    bool gold;     // true if coeff even
};

struct Cand {
    int s;   // square index 0..63
    int cc;  // coefficient at selection time (before placing)
};

static constexpr int N = 8;
// static constexpr int THEO_MIN = 2*N - 4; // provable lower bound for GOLDs
static array<array<uint64_t,4>, 64> RAY; // for each square s, 4 ray masks: NE,NW,SW,SE

inline int idx(int r,int c){ return r*8 + c; }
inline bool inside(int r,int c){ return r>=0 && r<8 && c>=0 && c<8; }

// Precompute 4 ray bitmasks for each square (top-left origin).
void build_rays(){
    for(int r=0;r<8;++r){
        for(int c=0;c<8;++c){
            int s = idx(r,c);
            uint64_t m;

            // NE: (r-1,c+1)
            m = 0;
            for(int rr=r-1, cc=c+1; inside(rr,cc); --rr, ++cc) m |= (1ULL<<idx(rr,cc));
            RAY[s][0] = m;

            // NW: (r-1,c-1)
            m = 0;
            for(int rr=r-1, cc=c-1; inside(rr,cc); --rr, --cc) m |= (1ULL<<idx(rr,cc));
            RAY[s][1] = m;

            // SW: (r+1,c-1)
            m = 0;
            for(int rr=r+1, cc=c-1; inside(rr,cc); ++rr, --cc) m |= (1ULL<<idx(rr,cc));
            RAY[s][2] = m;

            // SE: (r+1,c+1)
            m = 0;
            for(int rr=r+1, cc=c+1; inside(rr,cc); ++rr, ++cc) m |= (1ULL<<idx(rr,cc));
            RAY[s][3] = m;
        }
    }
}

// Count full coefficient (0..4) for square s vs occupancy bitboard occ.
inline int coeff_count(uint64_t occ, int s){
    int cnt = 0;
    cnt += (RAY[s][0] & occ) ? 1 : 0;
    cnt += (RAY[s][1] & occ) ? 1 : 0;
    cnt += (RAY[s][2] & occ) ? 1 : 0;
    cnt += (RAY[s][3] & occ) ? 1 : 0;
    return cnt;
}

// Independent naive coefficient using explicit ray marching (for cross-check)
inline int coeff_count_naive(uint64_t occ, int s){
    int r = s/8, c = s%8;
    int cnt = 0;
    const int dr[4] = {-1,-1, 1, 1};
    const int dc[4] = { 1,-1,-1, 1};
    for(int k=0;k<4;++k){
        int rr = r + dr[k], cc = c + dc[k];
        while(rr>=0 && rr<8 && cc>=0 && cc<8){
            int t = rr*8 + cc;
            if( (occ>>t) & 1ULL ){ cnt++; break; }
            rr += dr[k]; cc += dc[k];
        }
    }
    return cnt;
}

// Count how many empty squares would have odd coefficient for a given occupancy
inline int count_odds_after(uint64_t occ){
    int oddNext = 0;
    for(int r=0;r<8;++r){
        for(int c=0;c<8;++c){
            int t = idx(r,c);
            if( (occ>>t) & 1ULL ) continue;
            if( coeff_count(occ, t) & 1 ) ++oddNext;
        }
    }
    return oddNext;
}

struct RunResult {
    int golds = INT_MAX;
    array<array<char,8>,8> grid{}; // 'G' / 'S'
    vector<Step> steps;
};

// Verify by replaying the sequence: at each step recompute the coefficient
// from the current occupancy and check parity & stored value.
static bool verify_run(const RunResult& res){
    uint64_t occ = 0ULL;
    for(size_t i=0;i<res.steps.size();++i){
        int r = res.steps[i].r;
        int c = res.steps[i].c;
        int s = idx(r,c);
        if( (occ>>s) & 1ULL ) return false; // double placement
        int cc_naive = coeff_count_naive(occ, s);
        // Cross-check bitboard vs naive; if mismatched, fail fast
        int cc_fast = coeff_count(occ, s);
        if(cc_naive != cc_fast){
            cerr << "[VERIFY] mismatch at step " << (i+1)
                 << ": naive=" << cc_naive << ", fast=" << cc_fast << "\n";
            return false;
        }
        bool gold = (cc_naive % 2 == 0);
        if (gold != res.steps[i].gold) return false;
        if (cc_naive != res.steps[i].coeff) return false;
        occ |= (1ULL<<s);
    }
    return true;
}

// Report counts of GOLDs by (x-y) mod 3 classes and the inequality LHS values
static void print_mod3_gold_stats(const RunResult& res){
    int G[3] = {0,0,0};
    for(const auto& st : res.steps){
        if(!st.gold) continue;
        int x = st.c + 1;          // column 1..8
        int y = 8 - st.r;          // row 1..8 (bottom is 1)
        int chi = ( (x - y) % 3 + 3 ) % 3;
        G[chi]++;
    }
    cout << "Gold counts by (x-y) mod 3: G0=" << G[0]
         << ", G1=" << G[1]
         << ", G2=" << G[2] << "\n";
    int e1 = 2*(G[0]+G[2]) - G[1];
    int e2 = 2*(G[0]+G[1]) - G[2];
    int e3 = 2*(G[1]+G[2]) - G[0];
    cout << "Ineq LHS: 2(G0+G2)-G1=" << e1
         << ", 2(G0+G1)-G2=" << e2
         << ", 2(G1+G2)-G0=" << e3 << "\n";
}

struct Runner {
    mt19937_64 rng;
    int lookahead = 1;     // lookahead ply depth: 0=off, 1=1-ply, 2=2-ply, 3=3-ply, ...
    int sampleK   = 0;     // 0: check all candidates; >0: random sample size
    double epsilon = 0.15; // ε-greedy: with probability epsilon, ignore lookahead and pick random from bucket
    bool preferLowerCoeff = true; // tie-break among best lookahead by lower coeff (e.g., prefer 1 over 3 if odd)
    double tau = 0.0;      // softmax temperature (>0 enables softmax over scores)

    explicit Runner(uint64_t seed, int la=1, int sk=0, double eps=0.15, double tau_=0.0)
    : rng(seed), lookahead(la), sampleK(sk), epsilon(eps), tau(tau_) {}

    // Recursive scoring: at 'depth' plies ahead, return
    // sum over plies of "count of odd-parity empties" assuming at each ply we pick
    // the move that maximizes that count. Branching is limited by sampleK if >0.
    int score_deeper(uint64_t occ, int depth){
        if(depth <= 0) return 0;
        // build candidate pool = all empties
        vector<int> pool; pool.reserve(64);
        for(int r=0;r<8;++r){
            for(int c=0;c<8;++c){
                int t = idx(r,c);
                if( ((occ>>t) & 1ULL) == 0 ) pool.push_back(t);
            }
        }
        if(pool.empty()) return 0;
        if(sampleK>0 && (int)pool.size()>sampleK){
            shuffle(pool.begin(), pool.end(), rng);
            pool.resize(sampleK);
        }
        int best = -1;
        for(int t : pool){
            uint64_t occ2 = occ | (1ULL<<t);
            int here = count_odds_after(occ2);
            int rest = (depth>1) ? score_deeper(occ2, depth-1) : 0;
            int sc = here + rest;
            if(sc > best) best = sc;
        }
        return best;
    }

    RunResult run_once(){
        RunResult out;
        uint64_t occ = 0ULL; // bits of filled squares
        for(auto &row: out.grid) row.fill('.');
        out.steps.clear();
        int golds = 0;

        for(int step=0; step<64; ++step){
            // Build candidate lists (store square and current coefficient)
            vector<Cand> odds, evens; odds.reserve(32); evens.reserve(32);
            for(int r=0;r<8;++r){
                for(int c=0;c<8;++c){
                    int s = idx(r,c);
                    if( (occ >> s) & 1ULL ) continue;
                    int cc = coeff_count(occ, s);
                    if(cc & 1) odds.push_back({s, cc});
                    else       evens.push_back({s, cc});
                }
            }

            const vector<Cand>* bucket = !odds.empty() ? &odds : &evens;

            int pick = -1;
            int pickedCoeff = -1;

            // ε-greedy: with probability epsilon, ignore lookahead and pick random from bucket
            uniform_real_distribution<double> ur(0.0, 1.0);
            bool doRandom = (lookahead==0) ? true : (ur(rng) < epsilon);

            if(doRandom){
                uniform_int_distribution<int> dist(0,(int)bucket->size()-1);
                const Cand& ch = (*bucket)[dist(rng)];
                pick = ch.s;
                pickedCoeff = ch.cc;
            }else{
                // Optional sampling to speed up lookahead.
                vector<Cand> pool = *bucket;
                if(sampleK>0 && (int)pool.size()>sampleK){
                    shuffle(pool.begin(), pool.end(), rng);
                    pool.resize(sampleK);
                }

                struct Scored{ Cand cand; int score; };
                vector<Scored> scored; scored.reserve(pool.size());
                int maxScore = INT_MIN;

                for(const Cand& x : pool){
                    uint64_t occ2 = occ | (1ULL<<x.s);
                    int sc = score_deeper(occ2, lookahead); // general d-ply
                    scored.push_back({x, sc});
                    if(sc > maxScore) maxScore = sc;
                }

                if(tau > 0.0){
                    // Softmax over scores (numerically stabilized)
                    double shift = (double)maxScore;
                    vector<double> w; w.reserve(scored.size());
                    double sumw = 0.0;
                    for(const auto& s : scored){
                        double ww = exp( ( (double)s.score - shift ) / tau );
                        w.push_back(ww); sumw += ww;
                    }
                    uniform_real_distribution<double> ud(0.0, sumw);
                    double r = ud(rng);
                    size_t chosen = 0;
                    for(size_t i=0;i<w.size();++i){ if(r <= w[i]){ chosen = i; break; } r -= w[i]; }
                    const Cand& ch = scored[chosen].cand;
                    pick = ch.s; pickedCoeff = ch.cc;
                }else{
                    // Choose among best-scoring; tie-break by preferred coefficient (lower by default)
                    vector<Cand> bests; bests.reserve(scored.size());
                    for(const auto& s : scored) if(s.score == maxScore) bests.push_back(s.cand);
                    if(!bests.empty()){
                        int ref = bests.front().cc;
                        for(const auto& b : bests){
                            if(preferLowerCoeff) ref = min(ref, b.cc);
                            else                 ref = max(ref, b.cc);
                        }
                        vector<Cand> filt; filt.reserve(bests.size());
                        for(const auto& b : bests) if(b.cc == ref) filt.push_back(b);
                        uniform_int_distribution<int> dist(0,(int)filt.size()-1);
                        const Cand& ch = filt[dist(rng)];
                        pick = ch.s; pickedCoeff = ch.cc;
                    }else{
                        // Fallback (shouldn't happen): random from bucket
                        uniform_int_distribution<int> dist(0,(int)bucket->size()-1);
                        const Cand& ch = (*bucket)[dist(rng)];
                        pick = ch.s; pickedCoeff = ch.cc;
                    }
                }
            }
            int pr = pick/8, pc = pick%8;
            int cc = (pickedCoeff>=0) ? pickedCoeff : coeff_count(occ, pick);

            bool isGold = (cc % 2 == 0);
            if(isGold) ++golds;
            out.grid[pr][pc] = isGold ? 'G' : 'S';
            occ |= (1ULL<<pick);
            out.steps.push_back({pr, pc, cc, isGold});
        }

        out.golds = golds;
        return out;
    }
};

static void print_result(const RunResult& res){
    cout << "\nBest found: GOLDs N = " << res.golds
         << ", SILVERs = " << (64 - res.golds) << "\n\n";

    cout << "Board (G=Gold, S=Silver) — printed bottom row first:\n";
    for(int pr=1; pr<=8; ++pr){
        int rr = 8 - pr; // bottom->top
        cout << setw(2) << pr << "  ";
        for(int c=0;c<8;++c){
            cout << res.grid[rr][c] << (c==7?'\n':' ');
        }
    }
    cout << "     ";
    for(int c=1;c<=8;++c) cout << c << (c==8?'\n':' ');
    cout << "\n";

    cout << "Move list in two columns (Step | (Col,Row) | Coeff | Coin):\n";
    cout << "| Step | Coord   | Coef | Coin  | Step | Coord   | Coef | Coin  |\n";
    cout << "|------|---------|------|-------|------|---------|------|-------|\n";
    auto cell = [](const string& s, int w){
        string t = s; if((int)t.size()<w) t += string(w-(int)t.size(),' ');
        return t.substr(0,w);
    };

    auto row_out = [&](int i){
        auto fmt_one = [&](int k)->string{
            if(k >= (int)res.steps.size()) return "|      |         |      |       |";
            const auto& st = res.steps[k];
            int col = st.c + 1;
            int row = 8 - st.r;
            string coin = st.gold ? "Gold" : "Silver";
            char buf[64];
            snprintf(buf, sizeof(buf), "| %4d | (%d,%d)   | %4d | %s",
                     k+1, col, row, st.coeff, coin.c_str());
            string s(buf);
            // pad to same width
            if((int)s.size() < 31) s += string(31-(int)s.size(),' ');
            return s;
        };
        cout << fmt_one(i) << " " << fmt_one(i+1) << "|\n";
    };

    for(int i=0;i<(int)res.steps.size(); i+=2) row_out(i);
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    build_rays();

    long long restarts = 1500000000000000LL;
    uint64_t seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    int lookahead = 1;
    int sampleK = 0;
    double epsilon = 0.15; // default ε for ε-greedy
    int targetN = 11;      // stop early if we reach this many GOLDs or fewer
    double tau = 0.0;      // softmax temperature (0 disables)
    int threads = 0; // 0 => use hardware_concurrency

    if(argc>=2){
        long long rr = atoll(argv[1]);
        if(rr < 1) rr = 1;
        restarts = rr;
    }
    if(argc>=3) seed = stoull(argv[2]);
    if(argc>=4){
        int la = atoi(argv[3]);
        if(la < 0) la = 0;
        if(la > 6) la = 6;
        lookahead = la;
    }
    if(argc>=5) sampleK = max(0, atoi(argv[4]));
    if(argc>=6) epsilon = atof(argv[5]);
    if(argc>=7) targetN = atoi(argv[6]);
    if(argc>=8) tau = atof(argv[7]);
    if(argc>=9) threads = max(0, atoi(argv[8]));

    RunResult best; best.golds = INT_MAX;
    std::mutex best_mtx;
    std::mutex io_mtx;
    std::atomic<long long> next_id{0};
    std::atomic<int> bestGold{INT_MAX};
    std::atomic<bool> stop{false};

    int numThreads = threads > 0 ? threads : (int)std::thread::hardware_concurrency();
    if(numThreads <= 0) numThreads = 1;

    vector<thread> pool;
    pool.reserve(numThreads);

    for(int tid=0; tid<numThreads; ++tid){
        pool.emplace_back([&, tid]{
            // thread-local loop
            while(true){
                if(stop.load(std::memory_order_relaxed)) break;
                long long i = next_id.fetch_add(1, std::memory_order_relaxed);
                if(i >= restarts) break;

                uint64_t seed_i = splitmix64(seed ^ (uint64_t)i ^ (uint64_t)(1469598103934665603ULL + tid));
                Runner R(seed_i, lookahead, sampleK, epsilon, tau);
                RunResult cur = R.run_once();

                int curGold = cur.golds;
                int prevBest = bestGold.load(std::memory_order_relaxed);
                if(curGold < prevBest){
                    std::lock_guard<std::mutex> lock(best_mtx);
                    if(curGold < best.golds){
                        best = cur;
                        bestGold.store(curGold, std::memory_order_relaxed);
                        std::lock_guard<std::mutex> lk(io_mtx);
                        cout << "New best N = " << best.golds
                             << " (restart " << (i+1) << "/" << restarts
                             << ", thread " << tid << ")\n";
                        if(best.golds <= targetN){
                            cout << "Target reached (N <= " << targetN << "). Stopping early.\n";
                            stop.store(true, std::memory_order_relaxed);
                        }
                    }
                }
            }
        });
    }

    for(auto &th : pool) th.join();

    print_result(best);
    bool ok = verify_run(best);
    cout << (ok ? "Verification: OK" : "Verification: FAILED") << "\n";
    print_mod3_gold_stats(best);
    return 0;
}
