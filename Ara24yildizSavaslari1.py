import matplotlib.pyplot as plt
from ortools.sat.python import cp_model

def solve_star_battle_with_ortools(N, K, regions):
    model = cp_model.CpModel()
    
    # Create boolean variables x[i][j]
    x = [[model.NewBoolVar(f'x_{i}_{j}') for j in range(N)] for i in range(N)]
    
    # Row constraints
    for i in range(N):
        model.Add(sum(x[i][j] for j in range(N)) == K)
    
    # Column constraints
    for j in range(N):
        model.Add(sum(x[i][j] for i in range(N)) == K)
        
    # Region constraints
    max_region_id = max(max(row) for row in regions)
    cells_in_region = [[] for _ in range(max_region_id+1)]
    for i in range(N):
        for j in range(N):
            rid = regions[i][j]
            cells_in_region[rid].append(x[i][j])
    
    for rid in range(max_region_id+1):
        if cells_in_region[rid]:
            model.Add(sum(cells_in_region[rid]) == K)
    
    # No adjacency
    directions = [(-1, -1), (-1, 0), (-1, 1),
                  (0, -1),           (0, 1),
                  (1, -1),  (1, 0),  (1, 1)]
    
    for i in range(N):
        for j in range(N):
            for (di, dj) in directions:
                ni = i + di
                nj = j + dj
                if 0 <= ni < N and 0 <= nj < N:
                    model.Add(x[i][j] + x[ni][nj] <= 1)
    
    solver = cp_model.CpSolver()
    status = solver.Solve(model)
    
    if status == cp_model.OPTIMAL or status == cp_model.FEASIBLE:
        solution = [[solver.Value(x[i][j]) for j in range(N)] for i in range(N)]
        return solution
    else:
        return None

def plot_star_battle_solution(N, regions, solution):
    fig, ax = plt.subplots(figsize=(6,6))
    ax.set_xlim(0, N)
    ax.set_ylim(0, N)
    ax.set_aspect('equal')
    ax.invert_yaxis()
    
    # Draw light grid
    for x in range(N+1):
        ax.plot([x,x],[0,N], color='gray', linewidth=0.5)
    for y in range(N+1):
        ax.plot([0,N],[y,y], color='gray', linewidth=0.5)
    
    # Draw region boundaries
    # Horizontal boundaries
    for i in range(N-1):
        for j in range(N):
            if regions[i][j] != regions[i+1][j]:
                ax.plot([j,j+1],[i+1,i+1], color='black', linewidth=2)
    # Vertical boundaries
    for i in range(N):
        for j in range(N-1):
            if regions[i][j] != regions[i][j+1]:
                ax.plot([j+1,j+1],[i,i+1], color='black', linewidth=2)

    # Outer boundary
    ax.plot([0,N],[0,0], color='black', linewidth=2)
    ax.plot([0,N],[N,N], color='black', linewidth=2)
    ax.plot([0,0],[0,N], color='black', linewidth=2)
    ax.plot([N,N],[0,N], color='black', linewidth=2)
    
    # Place stars
    for i in range(N):
        for j in range(N):
            if solution[i][j] == 1:
                ax.text(j+0.5, i+0.5, '★', fontsize=16, ha='center', va='center', color='black')
    
    ax.axis('off')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Example usage:
    N = 10
    K = 2
    # Example regions
    regions = [
        [1,1,1,2,2,2,2,2,3,3],
        [2,2,2,2,2,2,2,2,3,3],
        [2,2,4,4,4,2,2,2,3,3],
        [5,2,2,7,4,6,6,6,3,3],
        [5,2,2,7,7,6,6,6,6,5],
        [5,5,2,2,7,7,7,6,6,8],
        [2,2,2,2,2,7,7,9,9,8],
        [10,10,10,10,10,7,9,9,8,8],
        [10,10,10,10,10,9,9,9,8,8],
        [10,10,10,10,9,9,9,9,9,8]
    ]

    # Solve
    solution = solve_star_battle_with_ortools(N, K, regions)
    if solution is not None:
        plot_star_battle_solution(N, regions, solution)
    else:
        print("No solution found.")