
import sys

def solve_kropki():
    # Grid size
    N = 6
    
    # Board: 0 means empty
    board = [[0] * N for _ in range(N)]
    
    # ---------------------------------------------------------
    # CONSTRAINT DEFINITIONS
    # User will provide these. Format: ((r1, c1), (r2, c2), type)
    # type: 'W' (White) or 'B' (Black)
    # Coordinates are 0-indexed (0 to 5)
    # Example: 
    # dots = [
    #     ((0, 0), (0, 1), 'W'), 
    #     ((2, 3), (3, 3), 'B')
    # ]
    # ---------------------------------------------------------
    
    # JSON Data provided by user
    import json
    user_json = """
[
  {
    "type": "W",
    "cells": [[0, 2], [0, 3]]
  },
  {
    "type": "W",
    "cells": [[0, 4], [0, 5]]
  },
  {
    "type": "W",
    "cells": [[1, 0], [1, 1]]
  },
  {
    "type": "W",
    "cells": [[1, 1], [1, 2]]
  },
  {
    "type": "W",
    "cells": [[2, 2], [2, 3]]
  },
  {
    "type": "W",
    "cells": [[2, 3], [2, 4]]
  },
  {
    "type": "W",
    "cells": [[2, 4], [2, 5]]
  },
  {
    "type": "W",
    "cells": [[3, 1], [3, 2]]
  },
  {
    "type": "W",
    "cells": [[4, 0], [4, 1]]
  },
  {
    "type": "W",
    "cells": [[5, 3], [5, 4]]
  },
  {
    "type": "B",
    "cells": [[0, 1], [1, 1]]
  },
  {
    "type": "B",
    "cells": [[0, 2], [1, 2]]
  },
  {
    "type": "W",
    "cells": [[0, 3], [1, 3]]
  },
  {
    "type": "W",
    "cells": [[0, 4], [1, 4]]
  },
  {
    "type": "W",
    "cells": [[2, 0], [3, 0]]
  },
  {
    "type": "W",
    "cells": [[2, 1], [3, 1]]
  },
  {
    "type": "B",
    "cells": [[2, 4], [3, 4]]
  },
  {
    "type": "B",
    "cells": [[2, 5], [3, 5]]
  },
  {
    "type": "B",
    "cells": [[3, 1], [4, 1]]
  },
  {
    "type": "B",
    "cells": [[3, 2], [4, 2]]
  },
  {
    "type": "B",
    "cells": [[3, 3], [4, 3]]
  },
  {
    "type": "W",
    "cells": [[3, 4], [4, 4]]
  },
  {
    "type": "W",
    "cells": [[4, 0], [5, 0]]
  },
  {
    "type": "W",
    "cells": [[4, 1], [5, 1]]
  },
  {
    "type": "W",
    "cells": [[4, 3], [5, 3]]
  },
  {
    "type": "W",
    "cells": [[4, 4], [5, 4]]
  }
]
    """
    
    data = json.loads(user_json)
    dots = []
    for item in data:
        p1 = tuple(item['cells'][0])
        p2 = tuple(item['cells'][1])
        dots.append((p1, p2, item['type']))
    
    # Organize constraints for faster lookup
    # constraints_mapMap[(r, c)] -> list of checks
    # but actually we check neighbor validity.
    
    # Explicit dots map: set of pairs that HAVE a dot
    dot_map = {} # ( (r1,c1), (r2,c2) ) -> type
    
    for p1, p2, type_ in dots:
        # Sort pairs to ensure canonical key
        if p1 > p2:
            p1, p2 = p2, p1
        dot_map[(p1, p2)] = type_

    def get_neighbors(r, c):
        for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            nr, nc = r + dr, c + dc
            if 0 <= nr < N and 0 <= nc < N:
                yield nr, nc

    def check_valid(r, c, val):
        # 1. Row/Col Uniqueness
        for i in range(N):
            if board[r][i] == val: return False
            if board[i][c] == val: return False
            
        # 2. Neighbors constraints
        for nr, nc in get_neighbors(r, c):
            neighbor_val = board[nr][nc]
            if neighbor_val == 0:
                continue # not filled yet, checking later behavior: OK for now? 
                # actually we must ensure that if we place 'val', it keeps possibility valid?
                # For standard backtracking, we usually check full validity against *assigned* variables.
            
            # Form canonical pair
            p1, p2 = (r, c), (nr, nc)
            if p1 > p2: p1, p2 = p2, p1
            
            has_dot = (p1, p2) in dot_map
            dot_type = dot_map.get((p1, p2))
            
            diff = abs(val - neighbor_val)
            is_ratio_2 = (val == 2 * neighbor_val) or (neighbor_val == 2 * val)
            
            if has_dot:
                if dot_type == 'W':
                    # White: diff must be 1
                    if diff != 1:
                        return False
                elif dot_type == 'B':
                    # Black: ratio must be 2
                    if not is_ratio_2:
                        return False
            else:
                # No dot: MUST NOT satisfy dot conditions
                # "All black and white dots are given"
                if diff == 1:
                    return False
                if is_ratio_2:
                    return False
                    
        return True

    def backtrack(idx):
        if idx == N * N:
            return True
        
        r, c = idx // N, idx % N
        
        # Try values 1 to 6
        for val in range(1, N + 1):
            if check_valid(r, c, val):
                board[r][c] = val
                if backtrack(idx + 1):
                    return True
                board[r][c] = 0
        return False

    def print_constraints():
        # Create a visual representation
        # Rows: 2 * N - 1
        # Cols: 2 * N - 1
        
        # We will print somewhat expanded:
        # P . P . P
        # .   .   .
        # P . P . P
        
        # Let's just iterate and print
        print("Constraints Visualization:")
        for r in range(N):
            # Print row horizontal links
            line = []
            for c in range(N):
                line.append(" . ")
                if c < N - 1:
                    p1, p2 = (r, c), (r, c+1)
                    if p1 > p2: p1, p2 = p2, p1
                    if (p1, p2) in dot_map:
                        dtype = dot_map[(p1, p2)]
                        sym = ' o ' if dtype == 'W' else ' * '
                        line.append(sym)
                    else:
                        line.append("   ")
            print("".join(line))
            
            # Print vertical links
            if r < N - 1:
                line_v = []
                for c in range(N):
                    p1, p2 = (r, c), (r+1, c)
                    if p1 > p2: p1, p2 = p2, p1
                    
                    if (p1, p2) in dot_map:
                        dtype = dot_map[(p1, p2)]
                        sym = ' o ' if dtype == 'W' else ' * '
                        line_v.append(sym)
                    else:
                        # line_v.append("   ")
                        line_v.append("   ")
                    
                    if c < N - 1:
                        line_v.append("   ")
                
                # Center the vertical dots?
                # The visual above: " . " is length 3.
                # Vertical dot should be under the " . "
                # Let's adjust spacing.
                # " . " is center. 
                pass
                
                # Simple printing
                #  .     .     .
                #  o     *     
                #  .     .     .
                
                row_str = ""
                for c in range(N):
                    p1, p2 = (r, c), (r+1, c)
                    if p1 > p2: p1, p2 = p2, p1
                    center = " "
                    if (p1, p2) in dot_map:
                        dtype = dot_map[(p1, p2)]
                        center = 'o' if dtype == 'W' else '*'
                    row_str += f" {center}    "
                print(row_str)

    print_constraints()
    
    def draw_board(board_to_draw, output_file="kropki_solution.png"):
        try:
            import matplotlib.pyplot as plt
            import matplotlib.patches as patches
        except ImportError:
            print("Matplotlib not installed.")
            return

        fig, ax = plt.subplots(figsize=(8, 8))
        
        # Grid settings
        ax.set_xlim(0, N)
        ax.set_ylim(N, 0)
        ax.set_aspect('equal')
        ax.axis('off')

        # Draw grid lines
        for i in range(N + 1):
            ax.plot([0, N], [i, i], color='black', linewidth=2) # Horizontal
            ax.plot([i, i], [0, N], color='black', linewidth=2) # Vertical

        # Draw constraints
        # dot_map keys are ((r1, c1), (r2, c2))
        for (p1, p2), dtype in dot_map.items():
            r1, c1 = p1
            r2, c2 = p2
            
            # Center of the dot
            # Cell (r, c) center can be thought of as (c+0.5, r+0.5)
            # The edge between (r, c) and (r, c+1) is at (c+1, r+0.5)
            # The edge between (r, c) and (r+1, c) is at (c+0.5, r+1)
            
            if r1 == r2: # Horizontal neighbor
                cx = max(c1, c2)
                cy = r1 + 0.5
            else: # Vertical neighbor
                cx = c1 + 0.5
                cy = max(r1, r2)
            
            fill_color = 'black' if dtype == 'B' else 'white'
            circle = patches.Circle((cx, cy), 0.1, facecolor=fill_color, edgecolor='black', zorder=10)
            ax.add_patch(circle)

        # Draw numbers
        if board_to_draw:
            for r in range(N):
                for c in range(N):
                    val = board_to_draw[r][c]
                    if val != 0:
                        ax.text(c + 0.5, r + 0.5, str(val), 
                                ha='center', va='center', fontsize=20, color='blue')

        plt.savefig(output_file, bbox_inches='tight')
        print(f"Saved visualization to {output_file}")
        plt.close()

    print("Attempting to solve with STRICT negative constraints...")
    solved = backtrack(0)
    
    if solved:
        print("Solution Found (Strict)!")
        for row in board:
            print(row)
        draw_board(board)
    else:
        print("No solution with STRICT constraints.")
        print("Attempting to solve ignoring negative constraints (checking only present dots)...")
        
        # Reset board
        board = [[0] * N for _ in range(N)]
        
        # Modify valid check to ignore 'else' block
        def check_valid_relaxed(r, c, val):
            for i in range(N):
                if board[r][i] == val: return False
                if board[i][c] == val: return False
            for nr, nc in get_neighbors(r, c):
                neighbor_val = board[nr][nc]
                if neighbor_val == 0: continue
                p1, p2 = (r, c), (nr, nc)
                if p1 > p2: p1, p2 = p2, p1
                has_dot = (p1, p2) in dot_map
                dot_type = dot_map.get((p1, p2))
                diff = abs(val - neighbor_val)
                is_ratio_2 = (val == 2 * neighbor_val) or (neighbor_val == 2 * val)
                
                if has_dot:
                    if dot_type == 'W':
                        if diff != 1: return False
                    elif dot_type == 'B':
                        if not is_ratio_2: return False
                # RELAXED: Do NOT check negative constraints
            return True

        # Temporarily swap check_valid
        original_check = check_valid
        check_valid = check_valid_relaxed # override
        
        if backtrack(0):
            print("Solution Found (Relaxed)!")
            for row in board:
                print(row)
            draw_board(board)
        else:
            print("No solution even with relaxed constraints.")
            draw_board(None) # Draw empty board with constraints for debugging

if __name__ == "__main__":
    solve_kropki()
