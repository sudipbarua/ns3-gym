from tcp_base import TcpEventBased

__author__ = "Piotr Gawlowicz"
__copyright__ = "Copyright (c) 2018, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "gawlowicz@tkn.tu-berlin.de"


class TcpNewReno(TcpEventBased):
    """docstring for TcpNewReno"""
    def __init__(self):
        super(TcpNewReno, self).__init__()

    def get_action(self, obs, reward, done, info):
        # unique socket ID
        socketUuid = obs[0]
        # TCP env type: event-based = 0 / time-based = 1
        envType = obs[1]
        # sim time in us
        simTime_us = obs[2]
        # unique node ID
        nodeId = obs[3]
        # current ssThreshold
        ssThresh = obs[4]
        # current contention window size
        cWnd = obs[5]
        # segment size
        segmentSize = obs[6]
        # number of acked segments
        segmentsAcked = obs[7]
        # estimated bytes in flight
        bytesInFlight  = obs[8]

        new_cWnd = 1
        new_ssThresh = 1

        # IncreaseWindow
        if (cWnd < ssThresh):
            # slow start
            if (segmentsAcked >= 1):
                new_cWnd = cWnd + segmentSize

        if (cWnd >= ssThresh):
            # congestion avoidance
            if (segmentsAcked > 0):
                adder = 1.0 * (segmentSize * segmentSize) / cWnd;
                adder = int(max (1.0, adder))
                new_cWnd = cWnd + adder

        # GetSsThresh
        new_ssThresh = int(max (2 * segmentSize, bytesInFlight / 2))

        # return actions
        actions = [new_ssThresh, new_cWnd]

        return actions
    

########## Sample code for DQN ##########
import torch
import torch.nn as nn  
import torch.optim as optim

class DQN(nn.Module):
    def __init__(self, input_dim, output_dim):
        super(DQN, self).__init__()
        self.fc1 = nn.Linear(input_dim, 128)
        self.fc2 = nn.Linear(128, 128)
        self.fc3 = nn.Linear(128, output_dim)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = self.fc3(x)
        return x

class TcpDqn(TcpEventBased):
    """docstring for TcpNewReno"""
    def __init__(self):
        super(TcpNewReno, self).__init__()
        self.dqn = DQN(input_dim=9, output_dim=2)  # Example dimensions
        self.optimizer = optim.Adam(self.dqn.parameters())
        self.criterion = nn.MSELoss()

    def get_action(self, obs, reward, done, info):
        state = torch.tensor(obs, dtype=torch.float32)
        with torch.no_grad():
            action = self.dqn(state).numpy()

        new_ssThresh, new_cWnd = action

        # Ensure the actions are valid
        new_ssThresh = max(2 * obs[6], new_ssThresh)
        new_cWnd = max(1, new_cWnd)

        # return actions
        actions = [int(new_ssThresh), int(new_cWnd)]

        return actions

    def train(self, state, action, reward, next_state, done):
        # Implement your training logic here
        pass