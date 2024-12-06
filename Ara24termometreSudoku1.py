def solve_thermometer_sudoku(board, regions, thermometers):
    """
    Solve a 6x6 Thermometer Sudoku.

    Parameters:
        board: A 6x6 list of lists with integers 1-6 or 0 for empty cells.
        regions: A list of six sets, each containing (row, col) tuples that define the region cells.
                 Example for a standard 6x6 (3x2 boxes):
                 regions = [
                    {(0,0),(0,1),(1,0),(1,1),(2,0),(2,1)}, # top-left box
                    {(0,2),(0,3),(1,2),(1,3),(2,2),(2,3)}, # ...
                    {(0,4),(0,5),(1,4),(1,5),(2,4),(2,5)},
                    {(3,0),(3,1),(4,0),(4,1),(5,0),(5,1)},
                    {(3,2),(3,3),(4,2),(4,3),(5,2),(5,3)},
                    {(3,4),(3,5),(4,4),(4,5),(5,4),(5,5)},
                 ]
        thermometers: A list of lists of coordinates representing each thermometer path.
                      Example:
                      thermometers = [
                        [(0,0),(0,1),(1,1)], # A thermometer starting in board[0][0], then board[0][1], then board[1][1]
                        [(2,3),(3,3),(4,3),(5,3)] # Another thermometer
                      ]
    """

    def is_valid(r, c, val):
        # Check row
        for col in range(6):
            if board[r][col] == val:
                return False

        # Check column
        for row in range(6):
            if board[row][c] == val:
                return False

        # Check region
        for region in regions:
            if (r, c) in region:
                # region found, check if val already in that region
                for (rr, cc) in region:
                    if board[rr][cc] == val:
                        return False
                break

        # Temporarily place val to check thermometers
        original = board[r][c]
        board[r][c] = val

        # Check thermometer constraints
        for thermo in thermometers:
            # Extract values from board for cells in this thermometer
            thermo_values = [board[rr][cc] for (rr, cc) in thermo]

            # Ignore if still zeros inside (not fully placed)
            if any(v == 0 for v in thermo_values):
                continue

            # Values must be strictly increasing
            for i in range(len(thermo_values) - 1):
                if thermo_values[i] >= thermo_values[i+1]:
                    # revert and fail
                    board[r][c] = original
                    return False

        # revert
        board[r][c] = original
        return True

    def backtrack():
        # Find the next empty cell
        for row in range(6):
            for col in range(6):
                if board[row][col] == 0:
                    # Try values 1 to 6
                    for val in range(1, 7):
                        if is_valid(row, col, val):
                            board[row][col] = val
                            if backtrack():
                                return True
                            # backtrack
                            board[row][col] = 0
                    return False
        return True  # no empty cell found, puzzle solved

    if backtrack():
        return True
    else:
        return False

# Example usage:
if __name__ == "__main__":
    # Example empty board (all zeros), you must fill this with initial givens
    board = [
        [0,0,0,0,0,0],
        [0,0,0,0,0,0],
        [0,0,0,0,0,0],
        [0,0,0,0,0,0],
        [0,0,0,0,0,0],
        [0,0,0,0,0,0]
    ]

    # Define your regions
    regions = [
        {(0,0),(0,1),(0,2),(1,0),(1,1),(1,2)},
        {(0,3),(0,4),(0,5),(1,3),(1,4),(1,5)},
        {(2,0),(2,1),(2,2),(3,0),(3,1),(3,2)},
        {(2,3),(2,4),(2,5),(3,3),(3,4),(3,5)},
        {(4,0),(4,1),(4,2),(5,0),(5,1),(5,2)},
        {(4,3),(4,4),(4,5),(5,3),(5,4),(5,5)},
    ]

    # Define your thermometers
    # Example: Just one thermometer with three cells
    thermometers = [
        [(3,0),(3,1),(2,1),(1,1),(1,2)],
        [(5,1),(5,2),(4,2),(4,3),(3,3),(2,3)],
        [(4,5),(4,4),(3,4),(3,5)],
        ]

    # Once you have the actual puzzle data and thermometers, place them here.
    # For instance, if the puzzle gives you some initial digits, set them in the board above.

    solved = solve_thermometer_sudoku(board, regions, thermometers)
    if solved:
        for row in board:
            print(row)
    else:
        print("No solution found.")
