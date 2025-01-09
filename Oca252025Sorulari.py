from ortools.sat.python import cp_model

def evaluate_expression(digits, merge_list, op_list):
    """
    Construct a numerical expression from:
      - digits: a string, e.g. "123456789"
      - merge_list: a list of 0/1 of length len(digits)-1 (obtained by solver.Value on BoolVars)
          0 means "separate" (an operator is placed), 
          1 means "merge" (the current digit extends the previous multi-digit number)
      - op_list: a list of length len(digits)-1, 
          each element in {0=+, 1=-, 2=*, 3=/, 4="no operator" if merged}

    Then evaluate it under normal operator precedence (multiplication and division first).
    Return a float if valid, or None if there is a division by zero.
    """
    # 1) Build the list of numbers from the merge pattern
    numbers = []
    current_num = digits[0]
    for i in range(len(digits) - 1):
        if merge_list[i] == 1:
            # Merge with the next digit
            current_num += digits[i+1]
        else:
            # End the current number and start a new one
            numbers.append(int(current_num))
            current_num = digits[i+1]
    # Last number
    numbers.append(int(current_num))

    # 2) Create the actual operator list (excluding merges):
    actual_ops = []
    for i in range(len(merge_list)):
        if merge_list[i] == 0:
            actual_ops.append(op_list[i])

    # 3) Evaluate with standard precedence:
    #    first handle * and / from left to right, then + and - from left to right.
    if not numbers:
        return None
    new_nums = [numbers[0]]
    new_ops  = []
    for i, op_val in enumerate(actual_ops):
        next_num = numbers[i+1]
        if op_val == 2:  # '*'
            new_nums[-1] = new_nums[-1] * next_num
        elif op_val == 3:  # '/'
            if next_num == 0:
                return None  # invalid (division by zero)
            new_nums[-1] = new_nums[-1] / next_num
        else:
            # + or -
            new_nums.append(next_num)
            new_ops.append(op_val)

    # 4) Handle + and - from left to right
    result = new_nums[0]
    for i, op_val in enumerate(new_ops):
        if op_val == 0:  # '+'
            result += new_nums[i+1]
        elif op_val == 1:  # '-'
            result -= new_nums[i+1]

    return result

def build_expression_string(digits, merge_list, op_list):
    """
    Build a human-readable string expression from digits, merges, and op codes.
    op codes: 0=+, 1=-, 2=*, 3=/, 4="none" (due to merge)
    """
    pieces = []
    current = digits[0]
    for i in range(len(digits) - 1):
        if merge_list[i] == 1:
            current += digits[i+1]
        else:
            pieces.append(current)
            if op_list[i] == 0:
                pieces.append("+")
            elif op_list[i] == 1:
                pieces.append("-")
            elif op_list[i] == 2:
                pieces.append("*")
            elif op_list[i] == 3:
                pieces.append("/")
            current = digits[i+1]
    pieces.append(current)
    return "".join(pieces)

class SearchFor2025(cp_model.CpSolverSolutionCallback):
    def __init__(self, merge_vars, op_vars, digits, solutions_container):
        super().__init__()
        self._merge_vars = merge_vars  # These are BoolVars
        self._op_vars = op_vars        # These are IntVars in [0..4]
        self._digits = digits
        self._solutions_container = solutions_container
        self._count = 0

    def on_solution_callback(self):
        # Convert BoolVars to 0/1 integers
        merges = [self.Value(mv) for mv in self._merge_vars]
        # Convert op_vars to integers
        ops = [self.Value(ov) for ov in self._op_vars]
        val = evaluate_expression(self._digits, merges, ops)
        if val is not None and abs(val - 2025.0) < 1e-9:
            expr_str = build_expression_string(self._digits, merges, ops)
            self._solutions_container.append(expr_str)
            self._count += 1

    def solution_count(self):
        return self._count

def solve_2025_with_ortools(
    digits,
    must_use_all=False,   # If True, must use +, -, *, / each at least once
    only_mul_div=False,   # If True, only '*' and '/' are allowed (apart from merges)
    verbose=False
):
    """
    digits: e.g. "123456789"
    must_use_all: if True, each operator {+, -, *, /} must appear at least once
    only_mul_div: if True, only '*' and '/' are allowed (except merges).
    Return a sorted list of distinct expressions that evaluate to 2025.
    """
    model = cp_model.CpModel()
    n = len(digits)

    #  merge_vars[i]: BoolVar
    #    True  => merge with next digit (no operator)
    #    False => separate (operator)
    merge_vars = [model.NewBoolVar(f"merge_{i}") for i in range(n-1)]

    # op_vars[i] in {0=+, 1=-, 2=*, 3=/, 4=no-operator-if-merged}
    op_vars = [model.NewIntVar(0, 4, f"op_{i}") for i in range(n-1)]

    # Enforce relationship between merge and op:
    # If merge_vars[i] is True, then op_vars[i] == 4 (no operator).
    # If merge_vars[i] is False, then op_vars[i] in {0,1,2,3}.
    for i in range(n-1):
        # merge_vars[i] is a BoolVar => True or False
        model.Add(op_vars[i] == 4).OnlyEnforceIf(merge_vars[i])
        model.Add(op_vars[i] <= 3).OnlyEnforceIf(merge_vars[i].Not())

    # If only_mul_div=True, then we only allow * or / when not merged
    # So if merge is False, op in {2,3}
    if only_mul_div:
        for i in range(n-1):
            # This enforces op_vars[i] != 0,1 whenever merge_vars[i] is False
            model.Add(op_vars[i] != 0).OnlyEnforceIf(merge_vars[i].Not())
            model.Add(op_vars[i] != 1).OnlyEnforceIf(merge_vars[i].Not())

    # If must_use_all=True, we ensure +, -, *, / each at least once
    if must_use_all:
        reif_plus  = []
        reif_minus = []
        reif_mult  = []
        reif_div   = []
        for i in range(n-1):
            is_plus  = model.NewBoolVar(f"is_plus_{i}")
            is_minus = model.NewBoolVar(f"is_minus_{i}")
            is_mult  = model.NewBoolVar(f"is_mult_{i}")
            is_div   = model.NewBoolVar(f"is_div_{i}")

            # is_plus = 1 iff op_vars[i] == 0
            model.Add(op_vars[i] == 0).OnlyEnforceIf(is_plus)
            model.Add(op_vars[i] != 0).OnlyEnforceIf(is_plus.Not())

            # is_minus = 1 iff op_vars[i] == 1
            model.Add(op_vars[i] == 1).OnlyEnforceIf(is_minus)
            model.Add(op_vars[i] != 1).OnlyEnforceIf(is_minus.Not())

            # is_mult = 1 iff op_vars[i] == 2
            model.Add(op_vars[i] == 2).OnlyEnforceIf(is_mult)
            model.Add(op_vars[i] != 2).OnlyEnforceIf(is_mult.Not())

            # is_div = 1 iff op_vars[i] == 3
            model.Add(op_vars[i] == 3).OnlyEnforceIf(is_div)
            model.Add(op_vars[i] != 3).OnlyEnforceIf(is_div.Not())

            reif_plus.append(is_plus)
            reif_minus.append(is_minus)
            reif_mult.append(is_mult)
            reif_div.append(is_div)

        # Each operator must appear at least once
        model.Add(sum(reif_plus)  >= 1)
        model.Add(sum(reif_minus) >= 1)
        model.Add(sum(reif_mult)  >= 1)
        model.Add(sum(reif_div)   >= 1)

    solver = cp_model.CpSolver()
    all_solutions = []
    callback = SearchFor2025(merge_vars, op_vars, digits, all_solutions)

    # We do not add a direct expression == 2025 constraint to the model,
    # because that's non-linear. We enumerate all valid assignments and
    # evaluate them in the callback to check if they produce 2025.
    solver.SearchForAllSolutions(model, callback)

    if verbose:
        print(f"Digits={digits}, must_use_all={must_use_all}, only_mul_div={only_mul_div}")
        print(f"Found {callback.solution_count()} solutions that evaluate to 2025.")

    return sorted(set(all_solutions))

if __name__ == "__main__":
    # Example usage:
    # 1) "123456789" (commonly said to have two solutions)
    sol1 = solve_2025_with_ortools("123456789", must_use_all=False, only_mul_div=False, verbose=True)
    print("Question 1 (123456789) solutions:")
    for s in sol1:
        print(" ", s)
    print("-----")

    # 2) "987654321" (commonly said to have four solutions)
    sol2 = solve_2025_with_ortools("987654321", must_use_all=False, only_mul_div=False, verbose=True)
    print("Question 2 (987654321) solutions:")
    for s in sol2:
        print(" ", s)
    print("-----")

    # 3) "123454321"
    sol3 = solve_2025_with_ortools("123454321", must_use_all=False, only_mul_div=False, verbose=True)
    print("Question 3 (123454321) solutions:")
    for s in sol3:
        print(" ", s)
    print("-----")

    # 4) "1122334455" with must_use_all=True (each operator +, -, *, / at least once)
    sol4 = solve_2025_with_ortools("1122334455", must_use_all=True, only_mul_div=False, verbose=True)
    print("Question 4 (1122334455) [Must use +,-,*,/] solutions:")
    for s in sol4:
        print(" ", s)
    print("-----")

    # 5) "1122334455" with only_mul_div=True (only '*' and '/')
    sol5 = solve_2025_with_ortools("1122334455", must_use_all=False, only_mul_div=True, verbose=True)
    print("Question 5 (1122334455) [Only * and /] solutions:")
    for s in sol5:
        print(" ", s)