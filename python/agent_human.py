
import random

import click

from game import Agent, GameState
from keyboard_listener import keyboard_input
from patchwork import Patchwork
from patchwork_game import PatchPlacement2idx, PatchworkState, decode

from render import Render


class AgentHuman(Agent):
    """AI player based on MCTS"""
    def __init__(self, game: Patchwork, player_id):
        super().__init__()
        self.game = game
        self.player_id = player_id
        self.render = Render()
        self.next_sew_action = None
        self.next_choose_action = None

    def choose(self, key):
        if key == "d":
            self.game.choose_next(1)
        elif key == "a":
            self.game.choose_next(-1)
        elif key == "e":
            self.game.choose_roll(1)
        elif key == "q":
            self.game.choose_roll(-1)
        elif key =="h":
            self.game.render_history = True
        elif key == "\n":
            action = self.game.choosable(self.game.cx, self.game.state.player(self.player_id))
            if action:
                self.next_choose_action = action
                return True
        
        self.render.render(self.game)

    def sew(self, key):
        if key == "w":
            self.game.move((-1, 0))
        elif key == "s":
            self.game.move((1, 0))
        elif key == "a":
            self.game.move((0, -1))
        elif key == "d":
            self.game.move((0, 1))
        
        elif key == "r":
            self.game.rotate()

        elif key =="h":
            self.game.render_history = True
        elif key == "\n":
            # check if action is valid
            if self.game.sewable(self.game.sx, self.game.sy, self.game.st, self.game.next_sew, self.game.state.player(self.player_id)):
                self.next_sew_action = PatchPlacement2idx[(
                    self.game.next_sew.id, self.game.sx, self.game.sy, self.game.st)]
                return True
            # accept the current choose
    
        self.render.render(self.game)

    def get_action(self, state: PatchworkState):
        self.game.render_history = False
        if state.phase == 'choose':
            self.op = 'choose'
            self.game.cx = self.game.cl = 0
            self.game.cur_patch_queue = state.cur_patch_queue
            self.render.render(self.game)
            keyboard_input(self.choose)
            return self.next_choose_action
        elif state.phase == 'sew':
            self.op = 'sew'
            self.game.sx = self.game.sy = self.game.st = 0
            self.game.next_sew = state.next_sew
            self.render.render(self.game)
            keyboard_input(self.sew)
            return self.next_sew_action
        else:
            return random.choice(state.legal_actions())