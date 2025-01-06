from ortools.sat.python import cp_model

def solve_sum_shading_ortools():
    # Puzzle size
    N = 8

    # Define the puzzle constraints:
    #   rowTargets = [3, 29, 19, 25, 8, 24, 8, 30]
    #   colTargets = [28, 17,  8,  9, 27, 21, 32, 4]
    rowTargets = [3, 29, 19, 25, 8, 24, 8, 30]
    colTargets = [28, 17,  8,  9, 27, 21, 32,  4]

    # The 8x8 grid
    grid = [
        [7, 6, 4, 4, 2, 1, 8, 5],  # row 0
        [4, 6, 4, 9, 9, 7, 7, 9],
        [1, 2, 7, 7, 8, 3, 7, 7],
        [9, 8, 5, 3, 2, 4, 4, 9],
        [4, 2, 5, 2, 8, 3, 6, 4],
        [6, 5, 8, 7, 2, 5, 5, 8],
        [1, 1, 8, 3, 8, 3, 5, 6],
        [4, 9, 4, 9, 9, 8, 9, 3],  # row 7
    ]

    # Create the model
    model = cp_model.CpModel()

    # Create binary decision variables x[r][c]
    x = []
    for r in range(N):
        row_vars = []
        for c in range(N):
            var = model.NewBoolVar(f"x_{r}_{c}")
            row_vars.append(var)
        x.append(row_vars)

    # Row constraints:
    # sum(x[r][c] * grid[r][c]) = rowTargets[r]  for each row r
    for r in range(N):
        model.Add(
            sum(x[r][c] * grid[r][c] for c in range(N)) == rowTargets[r]
        )

    # Column constraints:
    # sum(x[r][c] * grid[r][c]) = colTargets[c]  for each column c
    for c in range(N):
        model.Add(
            sum(x[r][c] * grid[r][c] for r in range(N)) == colTargets[c]
        )

    # Create a solver and solve
    solver = cp_model.CpSolver()
    
    # We want *all* solutions, not just one. So we define a solution printer.
    class SumShadingSolutionPrinter(cp_model.CpSolverSolutionCallback):
        def __init__(self, variables, rows, cols):
            super().__init__()
            self._vars = variables
            self._solution_count = 0
            self.rows = rows
            self.cols = cols

        def OnSolutionCallback(self):
            self._solution_count += 1
            print(f"=== Solution #{self._solution_count} ===")
            for r in range(self.rows):
                row_str = []
                for c in range(self.cols):
                    if self.Value(self._vars[r][c]) == 1:
                        row_str.append(str(grid[r][c]))
                    else:
                        row_str.append("X")
                print(" ".join(row_str))
            print()

        def SolutionCount(self):
            return self._solution_count

    solution_printer = SumShadingSolutionPrinter(x, N, N)
    
    # Invoke the search for all solutions
    solver.SearchForAllSolutions(model, solution_printer)

    # Print final summary
    print(f"Number of solutions found: {solution_printer.SolutionCount()}")

if __name__ == "__main__":
    solve_sum_shading_ortools()