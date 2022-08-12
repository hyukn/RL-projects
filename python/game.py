
from abc import ABC, abstractmethod

class GameState(ABC):
    def __init__(self):
        pass

    @abstractmethod
    def legal_actions(self):
        pass

    @abstractmethod
    def do_apply_action(self):
        pass

    @abstractmethod
    def is_terminal(self):
        pass

    @abstractmethod
    def returns(self):
        pass

    @abstractmethod
    def observe(self):
        pass

class Game(ABC):
    def __init__(self):
        pass

class Agent(ABC):
    def __init__(self):
        self.name = None

    @abstractmethod
    def get_action(self):
        pass
