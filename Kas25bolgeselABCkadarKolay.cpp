#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int N = 6;
constexpr int LETTER_COUNT = 4;
constexpr char BLANK = 'X';
constexpr std::array<char, LETTER_COUNT> LETTERS = {'A', 'B', 'C', 'D'};

constexpr std::array<char, N> TOP_CLUES   = {'D', 'A', '.', 'D', '.', '.'};
constexpr std::array<char, N> BOTTOM_CLUES = {'.', '.', '.', '.', '.', '.'};
constexpr std::array<char, N> LEFT_CLUES  = {'.', '.', '.', 'A', '.', 'A'};
constexpr std::array<char, N> RIGHT_CLUES = {'A', '.', 'C', 'C', '.', '.'};

constexpr std::array<std::array<int, N>, N> REGION_MAP = {{
    {{1, 1, 1, 1, 1, 2}},
    {{3, 1, 2, 2, 2, 2}},
    {{3, 3, 4, 5, 5, 2}},
    {{3, 4, 4, 6, 5, 5}},
    {{3, 4, 6, 6, 5, 5}},
    {{3, 4, 4, 6, 6, 6}},
}};

constexpr int REGION_COUNT = 6;

struct State {
    std::array<std::array<char, N>, N> grid{};          // 0 = unassigned, else letter or BLANK
    std::array<int, N> rowRemaining{};                  // cells yet to assign per row
    std::array<int, N> colRemaining{};                  // per column
    std::array<int, REGION_COUNT> regRemaining{};       // per region
    std::array<int, N> rowLetters{};                    // letters placed per row
    std::array<int, N> colLetters{};                    // per column
    std::array<int, REGION_COUNT> regLetters{};         // per region
    std::array<uint8_t, N> rowMissing{};                // bitmask of letters still needed
    std::array<uint8_t, N> colMissing{};
    std::array<uint8_t, REGION_COUNT> regMissing{};

    State() {
        for (auto &row : grid) {
            row.fill(0);
        }
        rowRemaining.fill(N);
        colRemaining.fill(N);
        regRemaining.fill(N);
        rowLetters.fill(0);
        colLetters.fill(0);
        regLetters.fill(0);
        constexpr uint8_t all_letters_mask = (1u << LETTER_COUNT) - 1u;
        rowMissing.fill(all_letters_mask);
        colMissing.fill(all_letters_mask);
        regMissing.fill(all_letters_mask);
    }
};

inline int region_index(int r, int c) {
    return REGION_MAP[r][c] - 1;
}

inline int popcount(uint8_t mask) {
    return __builtin_popcount(static_cast<unsigned>(mask));
}

inline uint8_t remove_bit(uint8_t mask, int idx) {
    return mask & static_cast<uint8_t>(~(1u << idx));
}

inline bool has_bit(uint8_t mask, int idx) {
    return mask & (1u << idx);
}

inline std::optional<char> first_letter_row(const State &state, int r) {
    for (char ch : state.grid[r]) {
        if (ch == 0 || ch == BLANK) {
            continue;
        }
        return ch;
    }
    return std::nullopt;
}

inline std::optional<char> first_letter_col(const State &state, int c) {
    for (int r = 0; r < N; ++r) {
        char ch = state.grid[r][c];
        if (ch == 0 || ch == BLANK) {
            continue;
        }
        return ch;
    }
    return std::nullopt;
}

inline std::optional<char> last_letter_row(const State &state, int r) {
    for (int c = N - 1; c >= 0; --c) {
        char ch = state.grid[r][c];
        if (ch == 0 || ch == BLANK) {
            continue;
        }
        return ch;
    }
    return std::nullopt;
}

inline std::optional<char> last_letter_col(const State &state, int c) {
    for (int r = N - 1; r >= 0; --r) {
        char ch = state.grid[r][c];
        if (ch == 0 || ch == BLANK) {
            continue;
        }
        return ch;
    }
    return std::nullopt;
}

bool check_row_clues(const State &state, int r) {
    char left = LEFT_CLUES[r];
    if (left != '.') {
        auto first = first_letter_row(state, r);
        if (!first || *first != left) {
            return false;
        }
    }
    char right = RIGHT_CLUES[r];
    if (right != '.') {
        auto last = last_letter_row(state, r);
        if (!last || *last != right) {
            return false;
        }
    }
    return true;
}

bool check_col_clues(const State &state, int c) {
    char top = TOP_CLUES[c];
    if (top != '.') {
        auto first = first_letter_col(state, c);
        if (!first || *first != top) {
            return false;
        }
    }
    char bottom = BOTTOM_CLUES[c];
    if (bottom != '.') {
        auto last = last_letter_col(state, c);
        if (!last || *last != bottom) {
            return false;
        }
    }
    return true;
}

std::vector<char> domain(const State &state, int r, int c) {
    if (state.grid[r][c] != 0) {
        return {state.grid[r][c]};
    }

    std::vector<char> result;
    int reg = region_index(r, c);

    int rowRemAfterBase = state.rowRemaining[r] - 1;
    int colRemAfterBase = state.colRemaining[c] - 1;
    int regRemAfterBase = state.regRemaining[reg] - 1;

    for (int idx = 0; idx < LETTER_COUNT; ++idx) {
        if (!has_bit(state.rowMissing[r], idx) || !has_bit(state.colMissing[c], idx) || !has_bit(state.regMissing[reg], idx)) {
            continue;
        }
        if (state.rowLetters[r] >= LETTER_COUNT || state.colLetters[c] >= LETTER_COUNT || state.regLetters[reg] >= LETTER_COUNT) {
            continue;
        }

        int rowNeeded = popcount(remove_bit(state.rowMissing[r], idx));
        int colNeeded = popcount(remove_bit(state.colMissing[c], idx));
        int regNeeded = popcount(remove_bit(state.regMissing[reg], idx));

        if (rowRemAfterBase < rowNeeded || colRemAfterBase < colNeeded || regRemAfterBase < regNeeded) {
            continue;
        }

        result.push_back(LETTERS[idx]);
    }

    int rowNeededBlank = popcount(state.rowMissing[r]);
    int colNeededBlank = popcount(state.colMissing[c]);
    int regNeededBlank = popcount(state.regMissing[reg]);

    if (rowRemAfterBase >= rowNeededBlank && colRemAfterBase >= colNeededBlank && regRemAfterBase >= regNeededBlank) {
        result.push_back(BLANK);
    }

    return result;
}

void apply(State &state, int r, int c, char value) {
    int reg = region_index(r, c);
    state.grid[r][c] = value;
    state.rowRemaining[r] -= 1;
    state.colRemaining[c] -= 1;
    state.regRemaining[reg] -= 1;

    if (value == BLANK) {
        return;
    }

    int idx = std::find(LETTERS.begin(), LETTERS.end(), value) - LETTERS.begin();
    state.rowMissing[r] = remove_bit(state.rowMissing[r], idx);
    state.colMissing[c] = remove_bit(state.colMissing[c], idx);
    state.regMissing[reg] = remove_bit(state.regMissing[reg], idx);
    state.rowLetters[r] += 1;
    state.colLetters[c] += 1;
    state.regLetters[reg] += 1;
}

void undo(State &state, int r, int c, char value) {
    int reg = region_index(r, c);

    if (value == BLANK) {
        // nothing to restore for missing sets
    } else {
        int idx = std::find(LETTERS.begin(), LETTERS.end(), value) - LETTERS.begin();
        state.rowMissing[r] |= (1u << idx);
        state.colMissing[c] |= (1u << idx);
        state.regMissing[reg] |= (1u << idx);
        state.rowLetters[r] -= 1;
        state.colLetters[c] -= 1;
        state.regLetters[reg] -= 1;
    }

    state.rowRemaining[r] += 1;
    state.colRemaining[c] += 1;
    state.regRemaining[reg] += 1;
    state.grid[r][c] = 0;
}

bool solve(State &state) {
    int bestR = -1, bestC = -1;
    std::vector<char> bestDom;
    bool hasEmpty = false;

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            if (state.grid[r][c] != 0) {
                continue;
            }
            auto dom = domain(state, r, c);
            if (dom.empty()) {
                return false;
            }
            if (!hasEmpty || dom.size() < bestDom.size()) {
                hasEmpty = true;
                bestDom = std::move(dom);
                bestR = r;
                bestC = c;
                if (bestDom.size() == 1) {
                    break;
                }
            }
        }
        if (hasEmpty && bestDom.size() == 1) {
            break;
        }
    }

    if (!hasEmpty) {
        for (int r = 0; r < N; ++r) {
            if (popcount(state.rowMissing[r]) != 0 || !check_row_clues(state, r)) {
                return false;
            }
        }
        for (int c = 0; c < N; ++c) {
            if (popcount(state.colMissing[c]) != 0 || !check_col_clues(state, c)) {
                return false;
            }
        }
        for (int reg = 0; reg < REGION_COUNT; ++reg) {
            if (popcount(state.regMissing[reg]) != 0) {
                return false;
            }
        }
        return true;
    }

    int r = bestR;
    int c = bestC;
    int reg = region_index(r, c);

    for (char val : bestDom) {
        apply(state, r, c, val);

        bool valid = true;

        if (state.rowLetters[r] > LETTER_COUNT || state.colLetters[c] > LETTER_COUNT || state.regLetters[reg] > LETTER_COUNT) {
            valid = false;
        }

        if (valid) {
            if (state.rowRemaining[r] < popcount(state.rowMissing[r]) ||
                state.colRemaining[c] < popcount(state.colMissing[c]) ||
                state.regRemaining[reg] < popcount(state.regMissing[reg])) {
                valid = false;
            }
        }

        if (valid && state.rowRemaining[r] == 0) {
            if (state.rowLetters[r] != LETTER_COUNT || popcount(state.rowMissing[r]) != 0 || !check_row_clues(state, r)) {
                valid = false;
            }
        }

        if (valid && state.colRemaining[c] == 0) {
            if (state.colLetters[c] != LETTER_COUNT || popcount(state.colMissing[c]) != 0 || !check_col_clues(state, c)) {
                valid = false;
            }
        }

        if (valid && state.regRemaining[reg] == 0) {
            if (state.regLetters[reg] != LETTER_COUNT || popcount(state.regMissing[reg]) != 0) {
                valid = false;
            }
        }

        if (valid && solve(state)) {
            return true;
        }

        undo(state, r, c, val);
    }

    return false;
}

} // namespace

int main() {
    State state;
    if (!solve(state)) {
        std::cerr << "No solution found.\n";
        return 1;
    }

    for (int r = 0; r < N; ++r) {
        for (int c = 0; c < N; ++c) {
            char ch = state.grid[r][c];
            std::cout << (ch == 0 ? BLANK : ch);
            if (c + 1 < N) {
                std::cout << ' ';
            }
        }
        std::cout << '\n';
    }

    return 0;
}