
import random

from game import Agent

class AgentRandom(Agent):
    """AI player based on MCTS"""
    def __init__(self):
        pass
    def get_action(self, state):
        return random.choice(state.legal_actions())