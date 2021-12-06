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
    with open(Observation_Path, 'w') as Observe:   
        Observe.close()
    with open(Action_Path, 'w') as Act:
        Act.close()
    with open(Test_Log_Path, 'w') as Tes:
        Tes.close()
    with open(UseTime_Log_Path, 'w') as Use:
        Use.close()

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

    while 1:
        score_queue = []
        action1=0
        done=0

        while not done: 
            eps=options.EPS_DECAY

            while os.path.getsize(Observation_Path)==0:   
                i=1

            begin=datetime.datetime.now()

            Observation_Data=np.loadtxt(Observation_Path)   #有时候会消耗相对较多时间

            mid=datetime.datetime.now()

            with open(Observation_Path, 'w') as Observe:
                Observe.close()

            #

            observation = np.array([Observation_Data[0], Observation_Data[1], Observation_Data[2]])

            done=Observation_Data[3]

            with open(Test_Log_Path, 'a') as Tes:
                Tes.write(str(action1)+'   '+str(Observation_Data[0])+'   '+str(Observation_Data[1])+'   '+str(Observation_Data[2])+'   '+str(done)+'\n')
                Tes.close()

            print action1

            #

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

            end=datetime.datetime.now()
            SpendTime=end-begin
            MidTime=mid-begin
            with open(UseTime_Log_Path, 'a') as TimeLog:
                TimeLog.write(str(begin)+'   '+str(end)+'   '+str(MidTime.total_seconds()*1000)+'ms'+'   '+str(SpendTime.total_seconds()*1000)+'ms'+'\n')
                TimeLog.close()

            while os.path.getsize(Action_Path)!=0:   
                i=1

            with open(Action_Path, 'a') as ActionLog:
                ActionLog.write(str(action1)+'   '+str(done))
                ActionLog.close()

if __name__ == "__main__":
    TrainOnLineGraph()
