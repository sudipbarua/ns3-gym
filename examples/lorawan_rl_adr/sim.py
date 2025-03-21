__author__ = "Sudip Barua"
__copyright__ = "Copyright (c) 2025, Technische Universität Chemnitz"
__version__ = "0.1.0"
__email__ = "sudip.barua@etit.tu-chemnitz.de"

# This file calls the ai_adr module and creates the NS3 environment from that
# So this module is the main entry point for the simulation
# This also create the agents based on the implementation in the ai_adr module
# Here we collect the observations/states, rewards, done and info from the environment
# Then we get the agent based on the observation and call the get_action method of the agent
# The agent returns the action based on the observation
# Then we call the step method of the environment with the action to send the action to the environment

from ai_adr import AiAdr
from ns3gym import ns3env
import argparse

parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=1,
                    help='Number of iterations, Default: 1')
args = parser.parse_args()
startSim = bool(args.start)
iterationNum = int(args.iterations) 

port = 5555
simTime = 10 # seconds
stepTime = 0.5  # seconds
seed = 12
simArgs = {"--duration": simTime}
debug = False

env = ns3env.Ns3Env(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
env.reset()

ob_space = env.observation_space
ac_space = env.action_space
print("Observation space: ", ob_space,  ob_space.dtype)
print("Action space: ", ac_space, ac_space.dtype)

stepIdx = 0
currIt = 0

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        reward = 0
        done = False
        info = {}
        print("Step: ", stepIdx)
        print("---obs: ", obs)

        while True:
            stepIdx += 1
            agent = AiAdr()
            agent.set_spaces(ob_space, ac_space)
            action = agent.get_action(obs, reward, done, info)
            print("---action: ", action)

            print("Step: ", stepIdx)
            obs, reward, done, info = env.step(action)
            print("---obs, reward, done, info: ", obs, reward, done, info)

            if done:
                break
        currIt += 1
        if currIt >= iterationNum:
            break
except KeyboardInterrupt:
    print("Ctrl+C pressed. Stopping the simulation...")
finally:
    env.close()
    print("Environment closed.")