
from asyncio import constants
import random
import copy
from itertools import product

import numpy as np

from game import Game, GameState

def decode(board):
    return np.array(list(np.binary_repr(board, SIZE*SIZE)), dtype=np.int8).reshape(SIZE, SIZE)

class Patch:
    def __init__(self, id, bc, tc, br, shape):
        self.id = id
        # shape in {0, 1} matrix
        self.shape = np.array(shape)
        # time cost
        self.tc = tc
        # button cost
        self.bc = bc
        # button reward
        self.br = br

        self.area = np.sum(shape)

        self.trans = None
        self.trans_num = None
        self.render = None

    @property
    def dim(self):
        return self.shape.shape

    def transpose(self):
        trans = []
        t = self.shape

        for _ in range(2):
            for i in range(4):
                t = np.rot90(t).tolist()
                if not any([t == cur for cur in trans]):
                    trans.append(t)

            t = np.array(t).transpose()

        self.trans_num = len(trans)
        self.trans = trans

        # render shape
        canvas = np.zeros((5, 3))
        for t in trans:
            if len(t[0]) <= 3:
                self.render = np.pad(t, ((5-len(t), 0), (0, 4-len(t[0]))))
                self.render[:, -1] = -1
                # self.render = np.concatenate([self.render, np.zeros()])
                if self.br > 0:
                    br_remain = self.br
                    for i, j in zip(*np.where(self.render == 1)):
                        self.render[i, j] = 11
                        br_remain -= 1
                        if br_remain == 0:
                            break
                break

        return trans

PatchLib = {
    11: Patch(11, 0, 0, 0, [[1]]),
    21: Patch(21, 2, 1, 0, [[1, 1]]),
    31: Patch(31, 2, 2, 0, [[1, 1, 1]]),
    32: Patch(32, 1, 3, 0, [[1, 1], [1, 0]]),
    33: Patch(33, 3, 1, 0, [[1, 1], [1, 0]]),
    41: Patch(41, 2, 2, 0, [[1, 1, 1], [0, 1, 0]]),
    42: Patch(42, 3, 2, 1, [[0, 1, 1], [1, 1, 0]]),
    43: Patch(43, 3, 3, 1, [[1, 1, 1, 1]]),
    44: Patch(44, 4, 2, 1, [[1, 1, 1], [1, 0, 0]]),
    45: Patch(45, 6, 5, 2, [[1, 1], [1, 1]]),
    46: Patch(46, 4, 6, 2, [[1, 1, 1], [1, 0, 0]]),
    47: Patch(47, 7, 6, 3, [[0, 1, 1], [1, 1, 0]]),
    51: Patch(51, 2, 2, 0, [[1, 1, 1], [1, 1, 0]]),
    52: Patch(52, 1, 2, 0, [[1, 1, 1], [1, 0, 1]]),
    53: Patch(53, 7, 1, 1, [[1, 1, 1, 1, 1]]),
    54: Patch(54, 3, 4, 1, [[1, 1, 1, 1], [0, 1, 0, 0]]),
    55: Patch(55, 2, 3, 1, [[0, 1, 1, 1], [1, 1, 0, 0]]),
    56: Patch(56, 10, 3, 2, [[1, 1, 1, 1], [1, 0, 0, 0]]),
    57: Patch(57, 5, 4, 2, [[0, 1, 0], [1, 1, 1], [0, 1, 0]]),
    58: Patch(58, 5, 5, 2, [[1, 0, 0], [1, 1, 1], [1, 0, 0]]),
    59: Patch(59, 10, 4, 3, [[0, 0, 1], [0, 1, 1], [1, 1, 0]]),
    61: Patch(61, 4, 2, 0, [[0, 1, 1, 1], [1, 1, 1, 0]]),
    62: Patch(62, 2, 1, 0, [[0, 0, 1, 0], [1, 1, 1, 1], [0, 1, 0, 0]]),
    63: Patch(63, 1, 2, 0, [[0, 0, 0, 1], [1, 1, 1, 1], [1, 0, 0, 0]]),
    60: Patch(60, 0, 3, 1, [[0, 1, 0, 0], [1, 1, 1, 1], [0, 1, 0, 0]]),
    64: Patch(64, 1, 5, 1, [[1, 1, 1, 1], [1, 0, 0, 1]]),
    65: Patch(65, 7, 4, 2, [[1, 1, 1, 1], [0, 1, 1, 0]]),
    66: Patch(66, 3, 6, 2, [[0, 1, 0], [1, 1, 1], [1, 0, 1]]),
    67: Patch(67, 7, 2, 2, [[1, 0, 0, 0], [1, 1, 1, 1], [1, 0, 0, 0]]),
    68: Patch(68, 10, 5, 3, [[1, 1, 1, 1], [1, 1, 0, 0]]),
    69: Patch(69, 8, 6, 3, [[0, 0, 1], [1, 1, 1], [1, 1, 0]]),
    71: Patch(71, 2, 3, 0, [[1, 1, 1], [0, 1, 0], [1, 1, 1]]),
    72: Patch(72, 1, 4, 1, [[0, 0, 1, 0, 0], [1, 1, 1, 1, 1], [0, 0, 1, 0, 0]]),
    81: Patch(81, 5, 3, 1, [[0, 1, 1, 0], [1, 1, 1, 1], [0, 1, 1, 0]]),
}

patch_init = sorted(list(PatchLib.keys()))[1:]
PatchLibTrans = dict()

for id, patch in PatchLib.items():
    PatchLibTrans[id] = patch.transpose()

PatchAlter = dict()

SIZE = 9
coord = np.indices((SIZE, SIZE)).transpose(1, 2, 0).reshape(-1, 2)
dot = 2**np.arange(SIZE * SIZE, dtype=np.object)

for id, alltrans in PatchLibTrans.items():
    PatchAlter[id] = dict()
    for t, trans in enumerate(alltrans):
        for x, y in coord:
            if x + len(trans) <= SIZE and y + len(trans[0]) <= SIZE:
                alter = np.zeros((SIZE, SIZE), dtype=np.object)
                alter[x:x+len(trans), y:y+len(trans[0])] += trans
                alter = alter.flatten()
                PatchAlter[id][x, y, t] = alter.dot(dot[::-1])

"""
Encodede each patch and its alternative candidate into binary 
"""

PatchAlterArray = list()
IdArray = list()
PatchPlacement2idx = dict()
PatchArea = list()
k = 0
for id, alters in PatchAlter.items():
    for (x, y, t), alter in alters.items():
        PatchAlterArray.append(PatchAlter[id][x, y, t])
        IdArray.append(id)
        PatchArea.append(PatchLib[id].area)
        PatchPlacement2idx[(id, x, y, t)] = k
        k += 1

PatchAlterArray = np.array(PatchAlterArray, dtype=np.object)
IdArray = np.array(IdArray, dtype=np.int32)
PatchArea = np.array(PatchArea, dtype=np.int32)

Square7Array = list()
for x, y in product(range(SIZE-7+1), range(SIZE-7+1)):
    alter = np.zeros((SIZE, SIZE), dtype=np.object)
    alter[x:x+7, y:y+7] = 1
    alter = alter.flatten()
    Square7Array.append(alter.dot(dot[::-1]))

Square7Array = np.array(Square7Array, dtype=np.object)

class Board:
    def __init__(self):
        # self.board = np.zeros((9, 9))
        self.br = 0
        self.area = 0
        self.pn = 0

    @property
    def score(self):
        # TODO: simplified with area
        return -162 + 2 * self.area

    def sew(self, x, y, patch: Patch):
        self.pn += 1
        self.area += np.sum(patch.shape)
        self.br += patch.br

    def sew_xy(self, x, y, patch: Patch):
        h, w = patch.dim
        self.board[y:y+h, x:x+w] += patch.shape
        self.br += patch.br

    def sewable_options(self, patch):
        pass

class TimeLine:
    SQUARE_PATCH_POS = [25, 31, 37, 43, 49]
    BUTTON_REWARD_POS = [4, 10, 16, 22, 28, 34, 40, 46, 52]
    LEN = 54
    def __init__(self):
        self.end = 53

        spp = 0
        brp = 0

        self.spp = list()
        self.brp = list()
        for i in range(self.end + 1):
            if brp < len(self.BUTTON_REWARD_POS) and i > self.BUTTON_REWARD_POS[brp]:
                brp += 1
            if spp < len(self.SQUARE_PATCH_POS) and i > self.SQUARE_PATCH_POS[spp]:
                spp += 1
            
            self.spp.append(spp)
            self.brp.append(brp)
        
tl = TimeLine()

class Player:
    __slots__ = ['id', 'cur_pos', 'prev_pos', '_buttons', 'square7', 'br', 'area', 'patch', 'board']
    INIT_BUTTONS = 5
    def __init__(self, id):
        self.id = id
        self.cur_pos = 0
        self.prev_pos = 0
        self._buttons = 5
        # self.board = Board()
        self.square7 = 0

        self.br = 0
        self.area = 0

        self.patch = list()
        self.board = 0

    @property
    def buttons(self):
        return self._buttons

    @property
    def is_terminal(self):
        return self.cur_pos == tl.end

    def move_reward(self, move):
        self._buttons += move

    def cross_spp(self, opponent):
        return (tl.spp[self.cur_pos] != tl.spp[self.prev_pos]) and (tl.spp[self.cur_pos] > tl.spp[opponent.cur_pos])
    
    def cross_brp(self):
        return tl.brp[self.cur_pos] != tl.brp[self.prev_pos]

    def choose(self, patch: Patch):
        self._buttons -= patch.bc

    def sew(self, id, pid):
        self.patch.append(pid)
        self.area += PatchLib[id].area
        self.br += PatchLib[id].br

        self.board = self.board | PatchAlterArray[pid]
    
    def move(self, move):
        self.prev_pos = self.cur_pos
        self.cur_pos = min(tl.end, self.cur_pos + move)

        # move cross brp
        if self.cross_brp():
            self._buttons += self.br

    def sewable(self, patch):
        alters = PatchAlterArray[IdArray == patch.id]
        s = self.board & alters
        return np.any(s == 0)

    def all_sewable_ways(self, patch):
        p = np.where(IdArray == patch.id)[0]
        alters = PatchAlterArray[p]
        s = self.board & alters
        return p[s == 0]

    # inefficient implementation
    def all_sewable_ways_ineff(self, patch: Patch):
        ways = list()
        for t, trans in enumerate(PatchLibTrans[patch.id]):
            for x, y in np.where(self.board == 0):
                if x + len(trans) < 9 and y + len(trans[0]) < 9:
                    if not np.any(self.board[x:x+len(trans), y:y+len(trans[0])] + patch.shape > 2):
                        ways.append((x, y, t))

    @property
    def future_score(self):
        return -162 + 2 * self.area + self.square7 + self.buttons + self.br * (9 - tl.brp[self.cur_pos])

    @property
    def score(self):
        return -162 + 2 * self.area + self.square7 + self.buttons

    @property
    def avg_score(self):
        return (self.future_score + 162) / self.cur_pos if self.cur_pos > 0 else 0

    def observe(self):
       return f"{self.id} {self.cur_pos} {self.buttons} {self.board} {str(self.patch)}"

    def __str__(self):
        s = f'ID: {self.id}\n'
        s += f'patch: {self.patch}\n'
        s += f'pos: {self.cur_pos}\n'
        s += f'score: {self.score}\n'
        s += f'br: {self.br}\n'
        s += f'area: {self.area}\n'
        s += f'buttons: {self.buttons}\n'
        s += f'board: \n'
        s += str(np.array(list(np.binary_repr(self.board, SIZE*SIZE))).reshape(SIZE, SIZE))
        s += '\n'
        return s

    def __copy__(self):
        c = Player(self.id)
        c.id = self.id
        c.cur_pos = self.cur_pos
        c.prev_pos = self.prev_pos
        c._buttons = self._buttons

        c.area = self.area
        c.br = self.br

        c.square7 = self.square7
        c.patch = [*self.patch]
        c.board = self.board

        return c

class PatchworkState(GameState):
    PATCH_NUM = 33
    post_node_init = np.array(list(range(1, PATCH_NUM)) + [0])
    pre_node_init = np.array([PATCH_NUM-1] + list(range(PATCH_NUM - 1)))
    def __init__(self):
        super().__init__()
        self.patch_queue = None
        self.cur_player, self.opponent = None, None
        self.phase = 'choose'
        self.square7 = False

        self.next_sew = None

        self.post_node = None
        self.pre_node = None
        self.cur_node = 0

        self.cur_patch = None

    def init(self, queue=None):
        self.cur_player, self.opponent = Player(0), Player(1)
        self.cur_player.board = 0
        self.opponent.board = 0
        if not queue:
            self.patch_queue = np.random.permutation(patch_init)
        else:
            self.patch_queue = queue

        self.post_node = np.array(self.post_node_init)
        self.pre_node = np.array(self.pre_node_init)
        self.cur_patch = np.full(len(IdArray), True)

    def legal_actions(self):
        if self.phase == 'choose':
            # TODO: shape is not concerned at this moment
            choose = []
            n = self.cur_node
            for i in range(3):
                patch = PatchLib[self.patch_queue[n]]
                if patch.bc <= self.cur_player.buttons and self.cur_player.sewable(patch):
                    choose.append((n, patch))
                n = self.post_node[n]

            # always add move forward action (-1, -1)
            choose.append((-1, -1))
            return choose

        elif self.phase == 'sew':
            # scan all the legal positions and different 
            # TODO: shape is not concerned at this moment, always return true
            # only returns 
            # for shape in PatchLibTrans[self.next_sew.id]:
            return self.cur_player.all_sewable_ways(self.next_sew)
        else:
            return [0]

    def do_apply_action(self, action):
        action_func = getattr(
            self, 'apply_' + self.phase + '_action',
            getattr(self, 'apply_default_action')
        )

        action_func(action)

    def apply_choose_action(self, action):
        if action == (-1, -1):
            move = self.opponent.cur_pos - self.cur_player.cur_pos + 1
            self.cur_player.move_reward(move)
            self.move(move)
        else:
            n, patch = action
            
            # choose this patch
            self.cur_player.choose(patch)
            post = self.post_node[n]
            pre = self.pre_node[n]

            # remove current patch from the queue
            self.post_node[pre] = post
            self.pre_node[post] = pre

            self.cur_node = post
            self.next_sew = patch
            self.phase = 'sew'

            # remove patches from IdArray
            self.cur_patch[IdArray == patch.id] = False
        
    def apply_sew_action(self, action):
        # action = start_point, patch
        # TODO: shape does not count at this moment
        id = IdArray[action]

        self.cur_player.sew(id, action)
        # if current player get square7:
        if not self.square7 and self.cur_player.area > 49:
            for b7 in Square7Array:
                if b7 & self.cur_player.board == b7:
                    self.cur_player.square7 = 7
                    self.square7 = True
                    break
        
        self.move(PatchLib[id].tc)

    def move(self, move):
        self.cur_player.move(move)

        # if player gets 1x1 patch, the first time a player cross
        if self.cur_player.cross_spp(self.opponent):
            self.phase = 'sew'
            self.next_sew = PatchLib[11]
        else:
            # if player moves to the front
            if self.cur_player.cur_pos > self.opponent.cur_pos:
                self.cur_player, self.opponent = self.opponent, self.cur_player

            # next choose phase
            self.phase = 'choose'

    def apply_default_action(self, action):
        pass
    
    @property
    def cur_patch_queue(self):
        nxt = self.post_node[self.cur_node]
        queue = [self.patch_queue[self.cur_node]]
        while nxt != self.cur_node:
            queue.append(self.patch_queue[nxt])
            nxt = self.post_node[nxt]
        
        return queue

    def player(self, player):
        return self.cur_player if self.cur_player.id == player else self.opponent

    def current_player(self):
        return self.cur_player.id
        
    def is_terminal(self):
        r = self.cur_player.is_terminal and self.opponent.is_terminal
        return self.cur_player.is_terminal and self.opponent.is_terminal

    def observe(self):
        if self.cur_player.id == 0:
            p0, p1 = self.cur_player, self.opponent
        else:
            p0, p1 = self.opponent, self.cur_player

        return p0.observe() + ' ' + p1.observe() + ' ' + str(self.cur_player.id)

    def returns(self, player):
        score = self.cur_player.score - self.opponent.score
        return score if self.cur_player.id == player else -score

    def future_returns(self, player):
        score = self.cur_player.future_score - self.opponent.future_score
        return score if self.cur_player.id == player else -score

    def avg_returns(self, player):
        score = self.cur_player.avg_score - self.opponent.avg_score
        return score if self.cur_player.id == player else -score

    def scores(self, player):
        return (self.cur_player.score, self.opponent.score) if self.cur_player.id == player else (self.opponent.score, self.cur_player.score) 

    def pos(self, player):
        return (self.cur_player.cur_pos, self.opponent.cur_pos) if self.cur_player.id == player else (self.opponent.cur_pos, self.cur_player.cur_pos) 
    

    def __str__(self):
        s = ''
        s += str(self.cur_player)
        s += str(self.opponent)

        return s

    def __copy__(self):
        c = PatchworkState()
        c.patch_queue = self.patch_queue
        c.phase = self.phase
        c.square7 = self.square7
        c.next_sew = self.next_sew
        c.patch_queue = self.patch_queue

        c.post_node = np.array(self.post_node)
        c.pre_node = np.array(self.pre_node)
        c.cur_node = self.cur_node
        c.cur_patch = np.array(self.cur_patch)

        c.cur_player, c.opponent = copy.copy(self.cur_player), copy.copy(self.opponent)

        return c

def main():
    np.random.seed(0)
    random.seed(0)

    state = PatchworkState()
    state.init()
    while not state.is_terminal():
        print(state)
        legal_actions = state.legal_actions()
        action = random.choice(legal_actions)
        state.do_apply_action(action)
        print(action)

    stop = 1
if __name__ == "__main__":
    main()