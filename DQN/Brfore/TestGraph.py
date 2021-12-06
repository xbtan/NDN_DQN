# -*- coding: utf-8 -*-    #兼容中文

import tensorflow as tf
import random
import numpy as np
from argparse import ArgumentParser
import os
import datetime

Observation_Path='../Data/Train/Observation.txt'
Action_Path='../Data/Train/Action.txt'
Test_Log_Path='../Data/Test/Test_Log.txt'
UseTime_Log_Path='../Data/Train_Log/UseTime_Log.txt'

import model1
import model2

def TrainOnLineGraph():
    tf.reset_default_graph()

    options = model1.get_options()
    agent = model2.QAgent(options)
    
    sess = tf.InteractiveSession()
    obs, Q1 = agent.add_value_net(options)
    act = tf.placeholder(tf.float32, [None, options.ACTION_DIM])
    rwd = tf.placeholder(tf.float32, [None, ])   
    next_obs, Q2 = agent.add_value_net(options)   
    
    values1 = tf.reduce_sum(tf.multiply(Q1, act), reduction_indices=1)
    values2 = rwd + options.GAMMA * tf.reduce_max(Q2, reduction_indices=1)
    loss = tf.reduce_mean(tf.square(values1 - values2))  
    train_step = tf.train.AdamOptimizer(options.LR).minimize(loss)  
        
    sess.run(tf.global_variables_initializer())

    variable_names = [v.name for v in tf.trainable_variables()]
    values = sess.run(variable_names)
    for k,v in zip(variable_names, values):
        print("Variable: ", k)
        print("Shape: ", v.shape)
        print(v)

    saver = tf.train.Saver()
    checkpoint = tf.train.get_checkpoint_state("../Graph")
    if checkpoint and checkpoint.model_checkpoint_path:
        saver.restore(sess, checkpoint.model_checkpoint_path)
        print("Successfully loaded:", checkpoint.model_checkpoint_path)
    else:
        print("Could not find old network weights")

    feed = {}
    eps = options.INIT_EPS
    global_step = 0
    exp_pointer = 0
    learning_finished = False

    obs_queue = np.empty([options.MAX_EXPERIENCE, options.OBSERVATION_DIM])
    act_queue = np.empty([options.MAX_EXPERIENCE, options.ACTION_DIM])
    rwd_queue = np.empty([options.MAX_EXPERIENCE])
    next_obs_queue = np.empty([options.MAX_EXPERIENCE, options.OBSERVATION_DIM])

    eps=options.EPS_DECAY

    Observation_Data=np.loadtxt(Observation_Path)

    observation = np.array([Observation_Data[0], Observation_Data[1], Observation_Data[2],Observation_Data[3]])

    action = agent.sample_action2(Q1, {obs : np.reshape(observation, (1, -1))}, eps, options)
    if action[0]==1.0:
        action1=1   
    else: 
        if action[1]==1.0:
            action1=2
        else: 
            if action[2]==1.0:
                action1=3
            else: 
                if action[3]==1.0:
                    action1=4
                else: 
                    if action[4]==1.0:
                        action1=5
                    else: 
                        if action[5]==1.0:
                            action1=6
                        else: 
                            if action[6]==1.0:
                                action1=7
    print action1

if __name__ == "__main__":
    TrainOnLineGraph()
