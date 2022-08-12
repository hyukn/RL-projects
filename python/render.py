
import random
import re
from matplotlib.pyplot import fill

import numpy as np
from click import clear, style, echo, get_current_context
from patchwork_game import PatchAlter, PatchPlacement2idx, PatchworkState, Player, TimeLine, decode, PatchLib, IdArray, PatchAlterArray, SIZE
from patchwork import Patchwork

random.seed(0)
# COLOURS = {
#     id: (random.randint(0,255), random.randint(0,255), random.randint(0,255)) 
#     for id, p in PatchLib.items()
# }

CELL_STYLE = {
    -1: " ",
    0: style('■', fg='black'),
    1: style('■', fg='white'),
    2: style('■', fg='green'),
    3: style('■', fg='red'),
    
    # square with button reward
    11: style('⧇', fg='white'),

    # cursor
    21: style('▲', fg="green"),
}

class Render:
    def __init__(self):        
        self.pos_marker = {
            0: style('►', fg="red"),
            1: style('►', fg="green"),
            "ol": style('►', fg="white"),
            "br": '○',
            "sp": '■',
        }
        # time line initial
        brp = np.array(TimeLine.BUTTON_REWARD_POS) * 2 + 1
        spp = np.array(TimeLine.SQUARE_PATCH_POS) * 2 + 1

        self.tl = [" " if i % 2 == 0 else '│' for i in range(2 * TimeLine.LEN)]
        for i in range(2 * TimeLine.LEN):
            if i in brp:
                self.tl[i] = self.pos_marker['br']
            elif i in spp:
                self.tl[i] = self.pos_marker['sp']

        # self.tl = "".join(self.tl)

        self.pass_render = np.full(shape=(5, 4), fill_value=3)
        self.pass_render[:, -1] = -1
        
    def pids2board(self, pids):
        boards = np.sum([decode(PatchAlterArray[p]) * IdArray[p] for p in pids]) + np.zeros((SIZE, SIZE))
        return boards

    def pids2sol(self, pids):
        sol = list()
        for pid in pids:
            id = IdArray[pid]
            place = PatchAlterArray[pid]
            place = decode(place)
            place = tuple(zip(*np.where(place == 1)))
            sol.append((id, place))
        
        return sol

    def render_cell(self, num):
        return CELL_STYLE[num]

    def render_player(self, player: Player, game: Patchwork):
        if game.state.phase == 'sew' and player.id == game.state.current_player():
            # render board according to operation in game
            board = decode(player.board)
            sx, sy, st = game.sx, game.sy, game.st
            # invalid pos
            if (game.next_sew.id, sx, sy, st) not in PatchPlacement2idx:
                board *= 3
            else:
                cur = decode(PatchAlter[game.next_sew.id][sx, sy, st]) * 2
                board += cur
        else:
            board = decode(player.board)
        yield f"{'ID':<4}{'Buttons':<8}{'Reward':<8}"
        yield f"{player.id:<4}{player.buttons:<8}{player.br:<8}"
        yield ""
        yield " " + " ".join(map(str, range(SIZE)))
        for x in range(SIZE):
            line_cells = (self.render_cell(board[x, y]) for y in range(SIZE))
            yield f"{x} " + " ".join(line_cells)
        
        # yield f"{game.sx, game.sy, game.st}"

    def render_timeline(self, game: Patchwork):
        p0, p1 = game.state.pos(0)
        yield '┌' + ('┬' + '─') * (TimeLine.LEN - 1)+ '┐'
        if p0 != p1:
            self.tl[p0 * 2] = self.pos_marker[0]
            self.tl[p1 * 2] = self.pos_marker[1]
            yield "".join(self.tl)
            self.tl[p0 * 2] = " "
            self.tl[p1 * 2] = " "
        else:
            self.tl[p0 * 2] = self.pos_marker['ol'] 
            yield "".join(self.tl)
            self.tl[p0 * 2] = " "

        # yield "".join([self.pos_marker[0] if i % 2 == 0 and i == 2 * p0 else " " for i in range(2 * TimeLine.LEN)])
        # yield "".join([self.pos_marker[1] if i % 2 == 0 and i == 2 * p1 else " " for i in range(2 * TimeLine.LEN)])
        yield '└' + ('┴' +'─') * (TimeLine.LEN - 1) + '┘'

    def render_lines(self, game: Patchwork):
        for _ in self.render_player(game.state.player(1), game):
            yield _
        for _ in self.render_player(game.state.player(0), game):
            yield _
        for _ in self.render_timeline(game):
            yield _
        
    def render_lines_concate(self, game: Patchwork):
        for a, b in zip(self.render_player(game.state.player(1), game), self.render_player(game.state.player(0), game)):
            a_s, b_s = a, b
            if '\x1b' in a:
                yield a_s + " " * 10 + b_s
            else:
                yield f"{a:<30}{b:<30}"

        for _ in self.render_timeline(game):
            yield _

        for _ in self.render_patch_queue(game):
            yield _

        if game.render_history:
            yield " ".join(game.history)

    def render_patch_queue(self, game: Patchwork):
        patch_queue = game.cur_patch_queue


        choose_list = [self.pass_render] + [PatchLib[p].render for p in patch_queue]
        s = game.cl
        e = min(len(patch_queue), s + 10)
        patch_lines_concate = np.concatenate(choose_list[s:e], axis=1)
        for l in patch_lines_concate:
            line_cells = [self.render_cell(c) for c in l]
            yield " ".join(line_cells)

        # tc and bc
        # '■ ■ ■   '
        info_list = ['pass    '] + [f'{PatchLib[p].tc}   {PatchLib[p].bc}   ' for p in patch_queue]
        info_line = "".join(info_list[s:e])

        # cursor line
        cursor_list = [' ' * 8 for _ in range(10)]
        cursor_list[game.cx + 1] = "".join([self.render_cell(c) for c in [-1, 21, 21, -1, -1, -1]])
        cursor_line = "".join(cursor_list[s:e])
        yield info_line
        yield cursor_line

    def render(self, game: Patchwork):
        clear()
        # echo("\n".join(self.render_lines(game)))
        echo("\n".join(self.render_lines_concate(game)))