from ortools.sat.python import cp_model

def solve_futoshiki():
    model = cp_model.CpModel()
    n = 5
    # Create 5x5 grid of integer variables between 1 and 5
    grid = [[model.NewIntVar(1, n, f'x_{i}_{j}') for j in range(n)] for i in range(n)]

    # All-different constraints for rows
    for i in range(n):
        for j in range(n):
            for k in range(j + 1, n):
                model.Add(grid[i][j] != grid[i][k])
    # All-different constraints for columns
    for j in range(n):
        for i in range(n):
            for k in range(i + 1, n):
                model.Add(grid[i][j] != grid[k][j])

    # Inequality constraints from the puzzle:
    # Vertical relations
    model.Add(grid[0][2] > grid[1][2])  # row0,col2 > row1,col2
    model.Add(grid[2][2] > grid[1][2])  # row2,col2 > row1,col2
    model.Add(grid[4][4] > grid[3][4])  # row4,col4 > row3,col4
    model.Add(grid[3][4] > grid[2][4])  # row4,col4 > row3,col4

    # Horizontal relations
    model.Add(grid[1][2] < grid[1][3])  # row1,col2 < row1,col3
    model.Add(grid[1][3] > grid[1][4])  # row1,col3 > row1,col4
    model.Add(grid[2][0] < grid[2][1])  # row2,col0 < row2,col1
    model.Add(grid[2][2] < grid[2][3])  # row2,col2 < row2,col3
    model.Add(grid[3][2] > grid[3][3])  # row3,col2 > row3,col3
    model.Add(grid[4][3] > grid[4][4])  # row4,col2 > row4,col3

    # Solve
    solver = cp_model.CpSolver()
    status = solver.Solve(model)
    if status in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        # Print full grid
        print("Solution:")
        for i in range(n):
            print([solver.Value(grid[i][j]) for j in range(n)])
        # Extract and print arrow sequences
        arrow1 = [solver.Value(grid[1][j]) for j in range(n)]
        arrow2 = [solver.Value(grid[3][j]) for j in range(n)]
        print(f"Arrow 1 (row 2): {arrow1}")
        print(f"Arrow 2 (row 4): {arrow2}")
    else:
        print("No solution found.")

if __name__ == '__main__':
    solve_futoshiki()