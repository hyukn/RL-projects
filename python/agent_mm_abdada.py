
import copy
from collections import OrderedDict
from itertools import product

import numpy as np

from game import GameState
# from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib
from patchwork_game import PatchAlterArray, PatchAlter, Patch, SIZE, IdArray, PatchPlacement2idx, PatchworkState, PatchLib, PatchArea, tl
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

class PatchworkChooseEvaluator:
    def __init__(self):
        self.memory = dict()
    
    def evaluate(self, patches, cur_pos):
        values = np.array([self.avg(patch, cur_pos) for(_, patch) in patches])
        return values

    # ignore the patches already been chosen
    def avg(self, patch: Patch, cur_pos):
        if patch == -1:
            return 1
        
        if patch.id == 11:
            return 2

        value = patch.br * (9 - tl.brp[cur_pos]) + 2 * patch.area - patch.bc
        return value / patch.tc

class PatchworkSewEvaluator:
    def __init__(self):
        self.memory = dict()
    
    def evaluate(self, board, placements, cur_patch):
        # regrets = np.array([self.pggr(board, pid, cur_patch) for pid in placements])
        regrets = np.array([self.pggr_partial(board, pid, cur_patch) for pid in placements])
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
        # # memorize
        # if (board, pid) in self.memory:
        #     return self.memory[(board, pid)]

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

        # self.memory[(board, pid)] = regret
        return regret

    @classmethod
    def rp(cls, board):
        s = board & PatchAlterArray
        Bij = len(s) - np.count_nonzero(s)
        return Bij / len(s)

import multiprocessing as mp
from multiprocessing import Pool, Process

import threading

class PatchworkMMABABDADASolver:
    def __init__(self, 
        placer=PatchworkPlacer.BL, 
        sew_evaluator=PatchworkSewEvaluator().evaluate,
        choose_evaluator=PatchworkChooseEvaluator().evaluate
    ):
        self.placer = placer
        self.sew_evaluator = sew_evaluator
        self.choose_evaluator = choose_evaluator
        self.branches = list()
        self.node_visits = 0

        # parallel
        self.DEFER_DEPTH = 1

    def defer_move(self, move_hash, depth):
        if depth < self.DEFER_DEPTH:
            return False

        return move_hash in self.move_hash_table

    def start_searching(self, move_hash, depth):
        if depth < self.DEFER_DEPTH:
            return 
        
        self.move_hash_table[move_hash] = True

    def solve_multi(self, state: GameState, depth, alpha, beta, player):
        manager = mp.Manager()
        pool = mp.Pool(int(mp.cpu_count()))
        # self.move_hash_table = manager.dict()
        # self.bound_table = manager.dict()
        
        self.bound_table = dict()
        self.move_hash_table = dict()

        # cores = 1
        # ps = [None for _ in range(cores)]
        # for i in range(1):
        #     ps[i] = pool.apply_async(self.alphabeta_dirty, args=(state, depth, alpha, beta, player))

        # # p = Process(target=self.alphabeta, args=(state, depth, alpha, beta, player))
        # # p.start()
        # results = [p.get() for p in ps]
        # max_result = max(results, key=lambda x: x[0])

        max_result = self.alphabeta_dirty(state, depth, alpha, beta, player)
        return max_result
        
    def solve(self, state: PatchworkState, depth, alpha, beta, player):
        self.bound_table = dict()
        self.move_hash_table = dict()
        return self.alphabeta(state, depth, alpha, beta, player)

    def alphabeta(self, state: PatchworkState, depth, alpha, beta, player):
        self.node_visits += 1
        if depth == 0 or state.is_terminal():
            avg = state.avg_returns(player)
            score = avg
            return score, None

        cur_player = state.cur_player
        # use heuristic method when the current phase is sew
        if state.current_player() == player:
            max_eval = -float('inf')
            max_action = None
            if state.phase == 'sew':
                # check for legal actions
                # TODO: accumulate all possible sew actions, validate and sew them after all choosing actions
                placements = self.placer(cur_player.board, state.next_sew)
                if len(placements) > 1:
                    regrets = self.sew_evaluator(cur_player.board, placements, state.cur_patch)
                    # sort regrets ascending, and only choose the best two placements
                    legal_actions = placements[np.argsort(regrets)[:4]]
                else:
                    legal_actions = placements

            elif state.phase == 'choose':
                # sort the choose actions
                chooses = state.legal_actions()
                values = self.choose_evaluator(chooses, state.cur_player.cur_pos)
                legal_actions = [chooses[i] for i in np.argsort(values)[::-1]]

            for i, action in enumerate(legal_actions):
                # PVS first move
                ob = state.observe()
                move_hash = ob + ' ' + str(action)
                if i == 0:
                    state_new = copy.copy(state)
                    state_new.do_apply_action(action)
                    eval, _ = self.alphabeta(state_new, depth, alpha, beta, player)
                    self.bound_table[ob] = alpha
                else:
                    if self.defer_move(move_hash, depth):
                        # update alpha from hash
                        # defered_move.append(action)
                        continue
                    else:
                        # mark the searching
                        if depth >= self.DEFER_DEPTH:
                            self.move_hash_table[move_hash] = True
                            # alpha = self.bound_table[ob]

                        # otherwise search the next move
                        state_new = copy.copy(state)
                        state_new.do_apply_action(action)
                        # zero window search
                        eval, _ = self.alphabeta(state_new, depth, alpha, alpha + 1, player)
                        # failure on zero window search
                        if alpha < eval < beta:
                            eval, _ = self.alphabeta(state_new, depth, alpha, beta, player)

                # # reread out alpha (probably updated by other processing)
                # alpha = self.bound_table.get(ob, alpha)
                # self.bound_table[ob] = max(alpha, eval)

                # also the max_eval 
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
            elif state.phase == 'choose':
                # sort the choose actions
                chooses = state.legal_actions()
                values = self.choose_evaluator(chooses, state.cur_player.cur_pos)
                legal_actions = [chooses[i] for i in np.argsort(values)[::-1]]

            for i, action in enumerate(legal_actions):
                # PVS first move
                ob = state.observe()
                move_hash = ob + ' ' + str(action)
                if i == 0:
                    state_new = copy.copy(state)
                    state_new.do_apply_action(action)
                    depth_new = depth - 1 if state_new.current_player() == player else depth
                    eval, _ = self.alphabeta(state_new, depth_new, alpha, beta, player)
                else:
                    if self.defer_move(move_hash, depth):
                        # update alpha from hash
                        # defered_move.append(action)
                        continue
                    else:
                        # mark the searching
                        if depth >= self.DEFER_DEPTH:
                            self.move_hash_table[move_hash] = True
                            # beta = self.bound_table[ob]

                        # otherwise search the next move
                        state_new = copy.copy(state)
                        state_new.do_apply_action(action)
                        depth_new = depth - 1 if state_new.current_player() == player else depth
                        # zero window search
                        eval, _ = self.alphabeta(state_new, depth_new, alpha, alpha + 1, player)
                        # failure on zero window search
                        if alpha < eval < beta:
                            eval, _ = self.alphabeta(state_new, depth_new, alpha, beta, player)

                # # reread out alpha (probably updated by other processing)
                # # bottle neck
                # # use 
                # beta = self.bound_table.get(ob, beta)
                # self.bound_table[ob] = min(beta, eval)

                if eval < min_eval:
                    min_eval = eval

                if beta <= alpha: 
                    break

            return min_eval, _

class AgentMMABDADA(object):
    def __init__(self, depth, player):
        super().__init__()
        self.depth = depth
        self.player = player

        # self.placer = PatchworkPlacer.paretoBL
        self.placer = PatchworkPlacer.BL
        # self.placer = PatchworkPlacer.BLLB
        self.sew_evaluator = PatchworkSewEvaluator().evaluate
        self.choose_evaluator = PatchworkChooseEvaluator().evaluate
        self.solver = PatchworkMMABABDADASolver(
            placer=self.placer, 
            sew_evaluator=self.sew_evaluator,
            choose_evaluator=self.choose_evaluator,
        )

    def get_action(self, state: PatchworkState):
        if len(state.legal_actions()) == 1:
            action = state.legal_actions()[0]
        else:
            max_eval, action = self.solver.solve_multi(
                state, self.depth, -float('inf'), float('inf'), self.player
            )
        return action

def pggr_test():
    ps = PatchworkSewEvaluator()
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