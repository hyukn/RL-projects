
import copy
from collections import OrderedDict
from itertools import product

import numpy as np

from game import GameState
# from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib
from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib, PatchArea
from patchwork_game import tl

class PatchworkPlacer:
    def __init__(self):
        # sort the key list in PatchAlter
        # create table to record the trial history / priority
        # self.BL_table = dict()
        # self.LB_table = dict()
        # self.paretoBL_table = dict()
        pass

    @classmethod
    def BL(cls, board, patch: Patch):
        # TODO: use PatchAlterArry, reorder it
        id = patch.id
        # xstart = cls.BL_table.get(id, 0)
        xstart = 0
        # use BL heuristic method
        for x, y, t in product(range(xstart, SIZE), range(SIZE), range(patch.trans_num)):
            if (x, y, t) in PatchAlter[id]:
                if board & PatchAlter[id][(x, y, t)] == 0:
                    # record the current x
                    return np.array([PatchPlacement2idx[(id, x, y, t)]], dtype=np.object)

    @classmethod   
    def LB(cls, board, patch: Patch):
        id = patch.id
        # ystart = cls.LB_table.get(id, 0)

        ystart = 0
        # use BL heuristic method
        for y, x, t in product(range(ystart, SIZE), range(SIZE), range(patch.trans_num)):
            if (x, y, t) in PatchAlter[id]:
                if board & PatchAlter[id][(x, y, t)] == 0:
                    # record the current y
                    return np.array([PatchPlacement2idx[(id, x, y, t)]], dtype=np.object)

    @classmethod
    def BLLB(cls, board, patch):
        return np.concatenate([cls.LB(board, patch), cls.BL(board, patch)])

    @classmethod
    def paretoBL(cls, board, patch):
        # keep x + y = B the same, scan for all possible combinatinons
        id = patch.id
        placements = list()
        for x in range(SIZE):
            for y in range(SIZE):
                found = False
                for t in range(patch.trans_num):
                    if (x, y, t) in PatchAlter[id]:
                        if board & PatchAlter[id][(x, y, t)] == 0:
                            placements.append(PatchPlacement2idx[(id, x, y, t)])
                            found = True
                if found:
                    break
        return np.array(placements, dtype=np.object)


class PatchworkAvgEvaluator:
    def __init__(self):
        self.memory = dict()
    
    def evaluate(self, patches, cur_pos):
        values = np.array([self.avg(patch, cur_pos) for(_, patch) in patches])
        return values

    # ignore the patches already been chosen
    def avg(self, patch: Patch, cur_pos):
        if patch == -1 or patch.id == 11:
            return 1

        value = patch.br * (9 - tl.brp[cur_pos]) + 2 * patch.area - patch.bc
        return value / patch.tc

class AgentHeu(object):
    def __init__(self, player):
        self.player = player
        self.placer = PatchworkPlacer.BL
        # self.placer = PatchworkPlacer.BL
        self.evaluator = PatchworkAvgEvaluator().evaluate

    def get_action(self, state: PatchworkState):
        if state.phase == 'sew':
            action = self.placer(state.cur_player.board, state.next_sew)[0]
        elif state.phase == 'choose':
            chooses = state.legal_actions()
            values = self.evaluator(chooses, state.cur_player.cur_pos)
            action = chooses[np.argsort(values)[-1]]
            
        return action
    
def main():
    pass

if __name__ == "__main__":
    main()