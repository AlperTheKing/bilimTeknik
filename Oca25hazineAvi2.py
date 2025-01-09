from ortools.sat.python import cp_model

def solve_diamond_puzzle(clues):
    R = len(clues)
    C = len(clues[0])
    
    model = cp_model.CpModel()
    
    # x[i][j] = 1 ise (i,j) hücresinde elmas var
    x = []
    for i in range(R):
        row_vars = []
        for j in range(C):
            row_vars.append(model.NewBoolVar(f"x_{i}_{j}"))
        x.append(row_vars)
    
    # 8 yönlü komşuluk
    def neighbors(r, c):
        for nr in range(r-1, r+2):
            for nc in range(c-1, c+2):
                if 0 <= nr < R and 0 <= nc < C and (nr != r or nc != c):
                    yield nr, nc

    # İpucu olan hücrelerde elmas yok; komşulardaki elmaslar clue değerine eşit
    for i in range(R):
        for j in range(C):
            clue = clues[i][j]
            if clue != -1 and clue is not None:  
                model.Add(x[i][j] == 0)
                model.Add(sum(x[nr][nc] for nr, nc in neighbors(i, j)) == clue)

    solver = cp_model.CpSolver()
    status = solver.Solve(model)
    
    if status in [cp_model.OPTIMAL, cp_model.FEASIBLE]:
        print("Çözüm bulundu:\n")
        solution_grid = []
        for i in range(R):
            row_solution = []
            for j in range(C):
                if solver.Value(x[i][j]) == 1:
                    row_solution.append("◆")  # Elmas
                else:
                    row_solution.append(".")   # Yok
            solution_grid.append(row_solution)
        
        for row in solution_grid:
            print(" ".join(row))
    else:
        print("Geçerli bir çözüm bulunamadı.")

if __name__ == "__main__":
    example_clues = [
        [-1, 2, -1, 1, -1, 2, -1, -1, 1, -1],
        [1, -1, -1,  2, 2, -1, 2, -1, 2, 2],
        [-1, 3, -1, -1, -1, 2, -1, 2, -1, 3],
        [2, -1, -1, -1, 3, -1, -1, -1, -1, -1],
        [-1, -1, 4, 3, 4, -1, -1, -1, 3, -1],
        [-1, 2, -1, -1, -1, 1, 1, 1, -1, -1],
        [-1, -1, -1, -1, -1, 2, -1, -1, -1, 2],
        [2, -1, 2, -1, 2, -1, -1, -1, 3, -1],
        [2, 3, -1, 2, -1, 2, 4, -1, -1, 2],
        [-1, 2, -1, -1, 1, -1, 2, -1, 2, -1]
    ]
    solve_diamond_puzzle(example_clues)