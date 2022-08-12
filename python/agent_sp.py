"""
Specifical angent for Patchwork
1. In the early game, only evaluate the choosen value for both player
2. Sew multiple 
"""
import copy
from collections import OrderedDict
from itertools import product

import numpy as np

from game import GameState
# from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib
from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib, PatchArea
from patchwork_game import decode

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


class PatchworkEvaluator:
    def __init__(self):
        self.memory = dict()
    
    def evaluate(self, board, placements, cur_patch):
        regrets = np.array([self.pggr(board, pid, cur_patch) for pid in placements])
        return regrets

    # ignore the patches already been chosen
    def pggr_partial(self, board, pid, cur_patch):
        if (board, pid) in self.memory:
            return self.memory[(board, pid)]

        id_alter = IdArray[cur_patch] != IdArray[pid]
        alters_other = PatchAlterArray[cur_patch][id_alter]
        s = board & alters_other        
        Bij = np.sum(PatchArea[cur_patch][id_alter][s == 0])
    
        # # TODO: ignore Bij when equals 1 here
        # valid = alters_other[s == 0]
        # reduce = np.sum(valid)
        # Bij = np.sum(reduce[reduce != 1])

        # calculate Bij_next
        board_next = board + PatchAlterArray[pid]
        s_next = board_next & alters_other
        Bij_next = np.sum(PatchArea[cur_patch][id_alter][s_next == 0])
        regret = Bij - Bij_next

        self.memory[(board, pid)] = regret
        return regret

    def pggr(self, board, pid, cur_patch):
        if (board, pid) in self.memory:
            return self.memory[(board, pid)]

        id_alter = IdArray != IdArray[pid]
        alters_other = PatchAlterArray[id_alter]
        s = board & alters_other        
        Bij = np.sum(PatchArea[id_alter][s == 0])
    
        # # TODO: ignore Bij when equals 1 here
        # valid = alters_other[s == 0]
        # reduce = np.sum(valid)
        # Bij = np.sum(reduce[reduce != 1])

        # calculate Bij_next
        board_next = board + PatchAlterArray[pid]
        s_next = board_next & alters_other
        Bij_next = np.sum(PatchArea[id_alter][s_next == 0])
        regret = Bij - Bij_next

        self.memory[(board, pid)] = regret
        return regret

    @classmethod
    def rp(cls, board):
        s = board & PatchAlterArray
        Bij = len(s) - np.count_nonzero(s)
        return Bij / len(s)

class PatchworkMMABSolver:
    def __init__(self, placer=PatchworkPlacer.BL, evaluator=PatchworkEvaluator().evaluate):
        self.placer = placer
        self.evaluator = evaluator

    # guarantee each player takes enough actions to evaluate
    # for instance: 
    # 1. sew must be filled after choosen.
    # 2. how to take button reward into consideration
    # assumes starting with player, end with the next player before switch back to current player

    def state_evalutate(self, state: PatchworkState, player):
        pass

    def solve(self, state: PatchworkState, depth, alpha, beta, player):
        if depth == 0 or state.is_terminal():
            # TODO: better evaluation and pruning
            # print(sd)
            avg = state.avg_returns(player)
            # dvp = state.returns(player)
            place = PatchworkEvaluator.rp(state.player(player).board)
            # score = place + dvp
            # score = avg
            # score = dvp
            score = place + avg
            return score, None

        cur_player = state.cur_player
        # use heuristic method when the current phase is sew

        if state.current_player() == player:
            max_eval = -float('inf')
            max_action = None
            if state.phase == 'sew':
                # check for legal actions
                placements = self.placer(cur_player.board, state.next_sew)
                if len(placements) > 1:
                    regrets = self.evaluator(cur_player.board, placements, state.cur_patch)
                    # sort regrets ascending, and only choose the best two placements
                    legal_actions = placements[np.argsort(regrets)[:2]]
                else:
                    legal_actions = placements
            else:
                legal_actions = state.legal_actions()

            for action in legal_actions:
                # state_new = copy.copy(state) if len(legal_actions) > 1 else state
                state_new = copy.copy(state)
                state_new.do_apply_action(action)
                eval, _ = self.solve(state_new, depth, alpha, beta, player)
                alpha = max(alpha, eval)

                if eval > max_eval:
                    max_action = action
                    max_eval = eval
                if beta <= alpha:
                    break
            
            return max_eval, max_action
        else:
            min_eval = float('inf')
            if state.phase == 'sew':
                legal_actions = PatchworkPlacer.BL(cur_player.board, state.next_sew)
            else:
                legal_actions = state.legal_actions()

            for action in legal_actions:
                # state_new = copy.copy(state) if len(legal_actions) > 1 else state
                state_new = copy.copy(state)
                state_new.do_apply_action(action)
                if state_new.current_player() == player:
                    eval, _ = self.solve(state_new, depth - 1, alpha, beta, player)
                else:
                    eval, _ = self.solve(state_new, depth, alpha, beta, player)
                beta = min(beta, eval)
                if eval < min_eval:
                    min_eval = eval

                if beta <= alpha: 
                    break

            return min_eval, _

class AgentMM(object):
    def __init__(self, depth, player):
        self.depth = depth
        self.player = player
        # self.placer = PatchworkPlacer.paretoBL
        self.placer = PatchworkPlacer.BL
        # self.placer = PatchworkPlacer.BLLB
        self.evaluator = PatchworkEvaluator().evaluate
        self.solver = PatchworkMMABSolver(placer=self.placer, evaluator=self.evaluator)

    def get_action(self, state: PatchworkState):
        if len(state.legal_actions()) == 1:
            action = state.legal_actions()[0]
        else:
            max_eval, action = self.solver.solve(
                state, self.depth, -float('inf'), float('inf'), self.player
            )
        return action

def pggr_test():
    ps = PatchworkEvaluator()
    board = 0
    rl = list()
    id = 60
    for (x, y, t), place in PatchAlter[id].items():
        pggr = ps.pggr(board, PatchPlacement2idx[(60, x, y, t)])
        rl.append(((x, y, t), pggr))
    
    rl = sorted(rl, key=lambda x: x[1])
    for (x, y, t), r in rl[:10]:
        print((x, y, t))
        print(decode(PatchAlter[60][(x, y, t)]))
    stop = 1
if __name__ == "__main__":
    pggr_test()