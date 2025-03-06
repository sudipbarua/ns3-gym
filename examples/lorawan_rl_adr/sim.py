# This file calls the ai_adr module and creates the NS3 environment from that
# So this module is the main entry point for the simulation
# This also create the agents based on the implementation in the ai_adr module
# Here we collect the observations/states, rewards, done and info from the environment
# Then we get the agent based on the observation and call the get_action method of the agent
# The agent returns the action based on the observation
# Then we call the step method of the environment with the action to send the action to the environment