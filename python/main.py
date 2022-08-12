
import random
import click
import graphviz

import numpy as np
import multiprocessing as mp
from multiprocessing import Pool, Process

from agent_mm_abdada import AgentMMABDADA, PatchworkMMABABDADASolver
from agent_mm_parallel import AgentMMP
from agent_mm_pvs import AgentMMPVS, PatchworkMMABPVSSolver
from agent_mm_pvs_parallel import AgentMMPVSParallel
from agent_mtd import AgentMTD

from game import GameState
from agent_random import AgentRandom
# from patchwork_agent import AgentMM, PatchworkMMABSolver, PatchworkEvaluator, PatchworkPlacer
# from patchwork_game import PatchworkGame, PatchworkState

from patchwork_game import Player, PatchworkState
from patchwork import Patchwork
from agent_mm import AgentMM, PatchworkMMABSolver, PatchworkSewEvaluator, PatchworkPlacer
from agent_heu import AgentHeu
from agent_human import AgentHuman
from patchwork_viz import PatchworkViz

def main_multi():
    rounds = 100
    # player0 = AgentMM(4, 0)
    # player0 = AgentMMP(3, 0)
    # player0 = AgentMMPVS(4, 0)

    # player0 = AgentMMPVS(4, 0)
    # player1 = AgentHeu(1)

    player0 = AgentMM(2, 0)
    player1 = AgentRandom()

    pv = PatchworkViz()
    log_level = 0
    pool = Pool(rounds)
    ps = list()
    for i in range(rounds):
        game = Patchwork()
        seed = i
        ps.append(pool.apply_async(game.play,
            args=(player0, player1, 0, seed)
        ))

    pool.close()
    pool.join()

    dvp = np.array([p.get() for p in ps])
    print(dvp)
    wr = len(dvp[dvp > 0]) / rounds
    print(f'winning ratio: {wr}')

def main():
    np.random.seed(0)
    random.seed(0)

    rounds = 1000
    win = 0

    game = Patchwork()

    player0 = AgentMM(1, 0)
    # player0 = AgentRandom()
    # player0 = AgentMMP(3, 0)
    # player0 = AgentMMPVS(3, 0)
    # player0 = AgentMMPVSParallel(1, 0)
    # player0 = AgentMMABDADA(1, 0)

    # player0 = AgentMTD(1, 0)
    # player0 = AgentHeu(0)

    # player1 = AgentMM(1, 1)
    # player1 = AgentRandom()
    # player1 = AgentHuman(game, 1)
    player1 = AgentHeu(1)
    # player1 = AgentRandom()
    pv = PatchworkViz()
    log_level = 0
    
    for i in range(rounds):
        # pv.draw_patch_queue(game.state.cur_patch_queue)
        DVP = game.play(player0, player1, log_level=0, seed=i)
        win = win + 1 if DVP > 0 else win
        print(f'No. {i}, DVP = {DVP}')
        # print(f'fail node = {player0.solver.fail_node}')
        # print(f"nodes = {player0.solver.node_visits}")
    print(f'winning ratio: {win / rounds}')

def profile():
    from line_profiler import LineProfiler
    lp = LineProfiler()
    lp.add_function(PatchworkMMABSolver.solve)

    lp_wrapper = lp(main)
    lp_wrapper()
    lp.print_stats()

if __name__ == "__main__":
    # profile()
    main()
    # main_multi()