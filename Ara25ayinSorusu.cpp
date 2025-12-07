#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

// Bitset for up to 704 bits (11 * 64 >= 666 constraints)
struct Bits {
  static constexpr int WORDS = 11;
  uint64_t w[WORDS];

  Bits() { std::memset(w, 0, sizeof(w)); }
  void set(int idx) { w[idx >> 6] |= 1ULL << (idx & 63); }
  bool test(int idx) const { return (w[idx >> 6] >> (idx & 63)) & 1ULL; }
  int count() const {
    int s = 0;
    for (int i = 0; i < WORDS; ++i)
      s += __builtin_popcountll(w[i]);
    return s;
  }
  void operator|=(const Bits &b) {
    for (int i = 0; i < WORDS; ++i)
      w[i] |= b.w[i];
  }
  Bits operator|(const Bits &b) const {
    Bits r;
    for (int i = 0; i < WORDS; ++i)
      r.w[i] = w[i] | b.w[i];
    return r;
  }
};

struct Solver {
  static constexpr int ROWS = 7;
  static constexpr int COLS = 7;
  static constexpr int CELLS = ROWS * COLS; // 49
  static constexpr int BLOCKS = 6 * 6;      // 36 different 2x2 blocks

  int constraintCount = 0; // 666
  Bits fullMask;           // 1s on used constraint bits
  std::vector<std::vector<int>> constraints; // squares per constraint
  std::vector<std::vector<int>> constraintSquares;
  std::vector<Bits> squareMasks; // constraints hit by a square

  explicit Solver(int threadsWanted)
      : threads(std::max(1u, threadsWanted == 0
                                  ? std::thread::hardware_concurrency()
                                  : static_cast<unsigned>(threadsWanted))) {
    buildConstraints();
  }

  std::optional<std::vector<int>> solve() {
    // Information-theoretic lower bound: 2^k - 1 >= 36 => k >= 6
    for (int limit = 6; limit <= CELLS; ++limit) {
      if (searchWithLimit(limit))
        return solution;
    }
    return std::nullopt;
  }

private:
  unsigned threads;
  std::atomic<bool> found{false};
  std::vector<int> solution;
  std::mutex solutionMutex;

  void buildConstraints() {
    // Build the 36 basic 2x2 blocks
    std::vector<std::array<int, 4>> blocks;
    for (int r = 0; r < ROWS - 1; ++r) {
      for (int c = 0; c < COLS - 1; ++c) {
        blocks.push_back(
            {r * COLS + c, r * COLS + c + 1, (r + 1) * COLS + c,
             (r + 1) * COLS + c + 1});
      }
    }

    // Constraint type 1: every block must be hit
    for (auto &b : blocks) {
      constraints.push_back({b[0], b[1], b[2], b[3]});
    }

    // Constraint type 2: each pair of blocks must differ
    for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
      for (int j = i + 1; j < static_cast<int>(blocks.size()); ++j) {
        std::vector<int> diff;
        for (int x : blocks[i])
          diff.push_back(x);
        for (int x : blocks[j]) {
          auto it = std::find(diff.begin(), diff.end(), x);
          if (it == diff.end())
            diff.push_back(x);
          else
            diff.erase(it);
        }
        constraints.push_back(diff);
      }
    }

    constraintCount = static_cast<int>(constraints.size()); // 666
    for (int i = 0; i < constraintCount; ++i)
      fullMask.set(i);

    constraintSquares = constraints;
    squareMasks.assign(CELLS, Bits{});
    for (int ci = 0; ci < constraintCount; ++ci) {
      for (int sq : constraints[ci]) {
        squareMasks[sq].set(ci);
      }
    }
  }

  // Intersection popcount
  static int intersectCount(const Bits &a, const Bits &b) {
    int s = 0;
    for (int i = 0; i < Bits::WORDS; ++i)
      s += __builtin_popcountll(a.w[i] & b.w[i]);
    return s;
  }

  Bits uncoveredFrom(const Bits &covered) const {
    Bits res = fullMask;
    for (int i = 0; i < Bits::WORDS; ++i)
      res.w[i] &= ~covered.w[i];
    return res;
  }

  int chooseConstraint(const Bits &covered) const {
    int best = -1;
    int bestSize = 1e9;
    for (int ci = 0; ci < constraintCount; ++ci) {
      if (!covered.test(ci)) {
        int sz = static_cast<int>(constraintSquares[ci].size());
        if (sz < bestSize) {
          bestSize = sz;
          best = ci;
          if (sz == 1)
            break;
        }
      }
    }
    return best;
  }

  bool dfs(Bits covered, int depth, int limit, std::vector<int> &current) {
    if (found.load(std::memory_order_relaxed))
      return true;
    if (covered.count() == constraintCount) {
      std::lock_guard<std::mutex> lock(solutionMutex);
      if (!found.load()) {
        solution = current;
        found.store(true);
      }
      return true;
    }
    if (depth == limit)
      return false;

    Bits uncovered = uncoveredFrom(covered);
    int rem = uncovered.count();
    int maxCov = 0;
    for (int sq = 0; sq < CELLS; ++sq) {
      int g = intersectCount(uncovered, squareMasks[sq]);
      if (g > maxCov)
        maxCov = g;
    }
    if (maxCov == 0)
      return false;
    int optimistic = (rem + maxCov - 1) / maxCov;
    if (depth + optimistic > limit)
      return false;

    int constraintIdx = chooseConstraint(covered);
    if (constraintIdx == -1)
      return false;

    // Order candidates by how many uncovered constraints they hit
    std::vector<std::pair<int, int>> candidates;
    for (int sq : constraintSquares[constraintIdx]) {
      int gain = intersectCount(uncovered, squareMasks[sq]);
      if (gain > 0)
        candidates.push_back({-gain, sq});
    }
    std::sort(candidates.begin(), candidates.end());

    for (auto [negGain, sq] : candidates) {
      if (found.load(std::memory_order_relaxed))
        return true;
      current.push_back(sq);
      Bits next = covered;
      next |= squareMasks[sq];
      dfs(next, depth + 1, limit, current);
      current.pop_back();
    }
    return false;
  }

  bool searchWithLimit(int limit) {
    found.store(false);
    solution.clear();

    int firstConstraint = 0; // any uncovered constraint works initially
    for (int i = 1; i < constraintCount; ++i) {
      if (constraintSquares[i].size() < constraintSquares[firstConstraint].size())
        firstConstraint = i;
    }
    std::vector<int> seeds = constraintSquares[firstConstraint];
    std::atomic<int> nextSeed{0};

    auto worker = [&]() {
      std::vector<int> localCurrent;
      while (!found.load()) {
        int idx = nextSeed.fetch_add(1);
        if (idx >= static_cast<int>(seeds.size()))
          break;
        int sq = seeds[idx];
        localCurrent.clear();
        localCurrent.push_back(sq);
        Bits covered;
        covered |= squareMasks[sq];
        dfs(covered, 1, limit, localCurrent);
      }
    };

    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (unsigned t = 0; t < threads; ++t)
      pool.emplace_back(worker);
    for (auto &th : pool)
      th.join();

    return found.load();
  }
};

int main(int argc, char **argv) {
  int threadCount = 0; // auto
  if (argc > 1)
    threadCount = std::max(1, std::atoi(argv[1]));

  Solver solver(threadCount);
  auto res = solver.solve();

  if (!res) {
    std::cout << "No solution found.\n";
    return 0;
  }

  std::vector<int> coins = *res;
  std::sort(coins.begin(), coins.end());
  std::cout << "Minimum coin count: " << coins.size() << "\n";
  std::cout << "Cell indices (0-based r*7+c):";
  for (int v : coins)
    std::cout << " " << v;
  std::cout << "\nPositions (row,col are 1-based):";
  for (int v : coins) {
    int r = v / 7, c = v % 7;
    std::cout << " (" << (r + 1) << "," << (c + 1) << ")";
  }
  std::cout << "\n";
  return 0;
}
