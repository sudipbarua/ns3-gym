__author__ = "Sudip Barua"
__copyright__ = "Copyright (c) 2025, Technische Universität Chemnitz"
__version__ = "0.1.0"
__email__ = "sudip.barua@etit.tu-chemnitz.de"

class AiAdr(object):
    def __init__(self):
        super(AiAdr, self).__init__()

    def set_spaces(self, obs, act):
        self.obsSpace = obs
        self.actSpace = act

    def get_action(self, obs, reward, done, info):
        print("obs: ", obs)
        
        # Dummy code for testing
        # In this section the AI agent will be implemented
        # The AI agent will take the observation as input and will return the action
        newTxPower = 11
        newDr = 3
        actions = [int(newDr), int(newTxPower)]
        return actions
