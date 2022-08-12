
import click
import numpy as np
import random

from game import Game
from patchwork_game import PatchworkState, SIZE, Player, Patch,  PatchPlacement2idx, PatchAlter, PatchLib
from patchwork_viz import PatchworkViz

class Patchwork(Game):
    def __init__(self, **kwargs):
        self.state = None
        # sew coord
        self.sx = self.sy = self.st = 0
        # choose id
        self.cx = -1
        self.next_sew = None
        self.render_next_sew = False
        self.cur_patch_queue = None
        self.render_history = False
        self.players = {0: None, 1: None}

    def init_game(self, seed=0):
        np.random.seed(seed)
        random.seed(seed)
        self.state = PatchworkState()
        self.state.init()
        self.history = []

    def play(self, player0, player1, log_level, seed):
        self.init_game(seed)
        self.players = {0: player0, 1: player1}
        while not self.state.is_terminal():
            player = self.state.current_player()
            action = self.players[player].get_action(self.state)

            if self.state.phase == 'sew':
                self.history.append(f"{player}-s-{self.state.next_sew.id} |")
            else:
                self.history.append(f"{player}-c-{(action[0], action[1].id) if not action[0] == -1 else 'pass'} |")
                
            self.state.do_apply_action(action)
            if log_level >= 1:
                print(self.state)


        return self.state.returns(0)

    def player(self, id):
        return self.players[id]
        
    def current_player(self):
        return self.state.current_player()

    def rotate(self):
        self.st = (self.st + 1) % self.next_sew.trans_num

    def move(self, move):
        self.sx = (self.sx + move[0]) % SIZE
        self.sy = (self.sy + move[1]) % SIZE

    def choose_next(self, nx):
        self.cx = max(min(2, self.cx + nx), -1)

    def choose_roll(self, nx):
        self.cl = max(min(len(self.cur_patch_queue) - 1, self.cl + nx), 0)

    def sewable(self, sx, sy, st, patch: Patch, player: Player):
        if (patch.id, sx, sy, st) not in PatchPlacement2idx:
            return False
        else:
            return player.board & PatchAlter[patch.id][sx, sy, st] == 0

    def choosable(self, choose_id, player: Player):
        if choose_id == -1:
            return (-1, -1)
        else:
            n = self.state.cur_node
            for i in range(choose_id):
                n = self.state.post_node[n]

            patch = PatchLib[self.state.patch_queue[n]]
            # patch = PatchLib[self.cur_patch_queue[choose_id]]
            if patch.bc <= player.buttons and player.sewable(patch):
                return (n, patch)
            else:
                return None