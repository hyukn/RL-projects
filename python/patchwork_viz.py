from itertools import product
import random
from matplotlib.pyplot import draw

import numpy as np

from svgwrite import Drawing
from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib, PatchArea
from patchwork_game import decode

_EMPTY = ' '

def solution_tiles(solution):
    """Generate all the tiles in a polyomino solution."""
    return (t for _, tiles in solution for t in tiles)

def solution_bounds(solution):
    """Return the bounds of the region covered by a polyomino solution."""
    h, w = map(max, zip(*solution_tiles(solution)))
    return h + 1, w + 1

class PatchworkViz:
    random.seed(0)
    COLOURS = {
        id: '#%02X%02X%02X' % (random.randint(0,255), random.randint(0,255), random.randint(0,255)) 
        for id, p in PatchLib.items()
    }
    def __init__(self):
        pass

    def pids2sol(self, pids):
        sol = list()
        for pid in pids:
            id = IdArray[pid]
            place = PatchAlterArray[pid]
            place = decode(place)
            place = tuple(zip(*np.where(place == 1)))
            sol.append((id, place))
        
        return sol

    def draw(self, sol):
        padding = 5
        size = 25
        drawing_size = (2 * padding + SIZE * size, 2 * padding + SIZE * size)
        drawing = Drawing(debug=False, filename='default.svg', size=drawing_size)
        self.draw_single_board(sol, drawing=drawing, oi=0, oj=0)
        drawing.save()

    def draw_history(self, sol):
        padding = 5
        size = 25
        columns = 1
        rows = len(sol)
        drawing_size = (2 * padding + SIZE * size, 2 * padding + (rows * (SIZE + 1) - 1) * size)
        drawing = Drawing(debug=False, filename='default.svg', size=drawing_size)
        for i in range(len(sol)):
            oi = padding + (i * (SIZE + 1)) * size
            self.draw_single_board(sol[:i + 1], drawing=drawing, oi=oi, oj=0, h=SIZE, w=SIZE)

        drawing.save()

    def draw_patch_queue(self, patch_queue):
        size = 25
        columns = 1
        padding = 0
        rows = len(patch_queue)
        drawing_size = (2 * padding + 4 * size, 2 * padding + (rows * (5 + 1) - 1) * size)
        drawing = Drawing(debug=False, filename='patch_queue.svg', size=drawing_size)
        for i in range(len(patch_queue)):
            patch = PatchLib[patch_queue[i]]
            oi = padding + (i * (5 + 1)) * size
            sol = [(patch_queue[i], tuple(zip(*np.where(patch.render > 0))))]
            self.draw_single_board(sol, drawing=drawing, oi=oi, oj=0, h=5, w=3)
            drawing.add(drawing.text(f"{patch.id} B={patch.bc}, T={patch.tc}", insert=(padding, oi + size), stroke='none',
                fill='black',
                font_size='15px',
                font_weight="bold",
                font_family="Arial"
            ))
        
        drawing.save()

    def draw_raw(self, solutions, filename, columns=1, size=25, padding=5,
                    colour=lambda _: "white", stroke_colour="black",
                    stroke_width=3):
        """Format polyomino tilings as an SVG image.

        Required arguments:
        solutions -- iterable of solutions to the tiling problem, each of
            which is a sequence of piece placements, each of which is a
            tuple whose first element is the name of the piece, and whose
            second element is a sequence of pairs (i, j) giving the
            locations of the tiles in the piece.
        filename -- where to save the SVG drawing.

        Optional arguments:
        columns -- number of solutions per row (default: 1).
        size -- width and height of each tile (default: 25).
        padding -- padding around the image (default: 5)
        colour -- function taking a piece name and returning its colour
            (default: a function returning white for each piece).
        stroke -- stroke colour (default: black).
        stroke_width -- width of strokes between pieces (default: 3).

        """
        solutions = list(solutions)
        h, w = solution_bounds(solutions[0])
        rows = (len(solutions) + columns - 1) // columns
        drawing_size = (2 * padding + (columns * (w + 1) - 1) * size,
                        2 * padding + (rows    * (h + 1) - 1) * size)
        drawing = Drawing(debug=False, filename=filename, size=drawing_size)
        for i, solution in enumerate(solutions):
            y, x = divmod(i, columns)
            oj = padding + (x * (w + 1)) * size
            oi = padding + (y * (h + 1)) * size
            group = drawing.g(stroke=stroke_colour, stroke_linecap="round",
                            stroke_width=0.25)
            drawing.add(group)
            grid = [[_EMPTY] * w for _ in range(h)]
            for c, placing in solution:
                piece = drawing.g(fill=colour(c))
                group.add(piece)
                for i, j in placing:
                    grid[i][j] = c
                    piece.add(drawing.rect((j * size + oj, i * size + oi),
                                        (size, size)))
            edges = drawing.path(stroke_width=stroke_width)
            group.add(edges)
            for i, j in product(range(h + 1), range(w)):
                if ((_EMPTY if i == 0 else grid[i-1][j])
                    != (_EMPTY if i == h else grid[i][j])):
                    edges.push(['M', j * size + oj, i * size + oi, 'l', size, 0])
            for i, j in product(range(h), range(w + 1)):
                if ((_EMPTY if j == 0 else grid[i][j-1])
                    != (_EMPTY if j == w else grid[i][j])):
                    edges.push(['M', j * size + oj, i * size + oi, 'l', 0, size])

        drawing.save()
    
    def draw_single_board(self, sol,
        drawing,
        oi=0,
        oj=0,
        stroke_colour="black",
        size=25,
        stroke_width=3,
        h=1,
        w=1,
    ):  
        # h, w = h, w
        # drawing = Drawing(debug=False, filename=filename, size=drawing_size)
        group = drawing.g(stroke=stroke_colour, stroke_linecap="round",
                        stroke_width=0.25)
        drawing.add(group)
        grid = [[_EMPTY] * w for _ in range(h)]
        for c, placing in sol:
            piece = drawing.g(fill=self.COLOURS[c])
            group.add(piece)
            for i, j in placing:
                grid[i][j] = c
                piece.add(drawing.rect((j * size + oj, i * size + oi),
                                    (size, size)))
        edges = drawing.path(stroke_width=stroke_width)
        group.add(edges)
        for i, j in product(range(h + 1), range(w)):
            if ((_EMPTY if i == 0 else grid[i-1][j])
                != (_EMPTY if i == h else grid[i][j])):
                edges.push(['M', j * size + oj, i * size + oi, 'l', size, 0])
        for i, j in product(range(h), range(w + 1)):
            if ((_EMPTY if j == 0 else grid[i][j-1])
                != (_EMPTY if j == w else grid[i][j])):
                edges.push(['M', j * size + oj, i * size + oi, 'l', 0, size])


def main():
    pv = PatchworkViz()
    # pids = [5776, 6512, 7767, 1146, 3611, 0, 5392, 330, 983, 2037, 46, 157, 6195, 8, 4673, 2117, 2567, 764]
    pids = [5552, 5844, 4542, 6288, 7557, 0, 7422, 5257, 283, 3173, 4876, 36, 173, 2948, 7089, 14]
    patch_queue = [51, 60, 64, 57, 32, 65, 66, 56, 47, 81, 69, 63, 55, 45, 53, 71, 42, 54, 68, 62, 31, 52, 43, 41, 58, 61, 59, 46, 44, 72, 67, 33, 21]
    # pv.draw(pv.pids2sol(pids))
    # pv.draw_history(pv.pids2sol(pids))
    pv.draw_patch_queue(patch_queue)

if __name__ == "__main__":
    main()