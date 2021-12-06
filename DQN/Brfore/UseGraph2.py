# -*- coding: utf-8 -*-    #兼容中文

import tensorflow as tf
import random
import numpy as np
from argparse import ArgumentParser
import os
import datetime

Observation_Path='../Data/Train/Observation.txt'
Action_Path='../Data/Train/Action.txt'

Train_Log_Path='../Data/Train_Log/Train_Log.txt'
Score_Log_Path='../Data/Train_Log/Score_Log.txt'
TrainTime_Log_Path='../Data/Train_Log/TrainTime_Log.txt'

import model1
import model2

def TrainOnLineGraph():
    with open(Observation_Path, 'w') as Observe:   
        Observe.close()
    with open(Action_Path, 'w') as Act:
        Act.close()
    with open(Train_Log_Path, 'w') as Tra:
        Tra.close()
    with open(Score_Log_Path, 'w') as Sco:
        Sco.close()
    with open(TrainTime_Log_Path, 'w') as Tim:
        Tim.close()

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
    checkpoint = tf.train.get_checkpoint_state("../Graph/Base")
    if checkpoint and checkpoint.model_checkpoint_path:
        saver.restore(sess, checkpoint.model_checkpoint_path)
        print("Successfully loaded:", checkpoint.model_checkpoint_path)
        OldGraph=True
    else:
        print("Could not find old network weights")
        OldGraph=False

    feed = {}
    eps = options.INIT_EPS
    global_step = 0
    exp_pointer = 0
    learning_finished = False

    obs_queue = np.empty([options.MAX_EXPERIENCE, options.OBSERVATION_DIM])
    act_queue = np.empty([options.MAX_EXPERIENCE, options.ACTION_DIM])
    rwd_queue = np.empty([options.MAX_EXPERIENCE])
    next_obs_queue = np.empty([options.MAX_EXPERIENCE, options.OBSERVATION_DIM])

    score_queue = []
    Score_Last_Eps=0

    for i_episode in range(options.MAX_EPISODE):   
        observation=np.array([1,1,10,0,20])
        action=np.zeros(options.ACTION_DIM)
        action[0]=1
        action1=0
        done=0
        score=0
        local_step=0
        sum_loss_value=0

        while not done:
            global_step += 1
            if global_step>=1000000:
                global_step=options.MAX_EXPERIENCE
            #print global_step

            local_step+=1
            if global_step % options.EPS_ANNEAL_STEPS == 0 and eps > options.FINAL_EPS:   
                eps = eps * options.EPS_DECAY
            
            obs_queue[exp_pointer] = observation   
            act_queue[exp_pointer] = action           

            while os.path.getsize(Observation_Path)==0:   
                i=1
            
            begin=datetime.datetime.now()

            Observation_Data=np.loadtxt(Observation_Path)
            with open(Observation_Path, 'w') as Observe:
                Observe.close()

            observation = np.array([Observation_Data[0], Observation_Data[1], Observation_Data[2],Observation_Data[3],Observation_Data[4]])

            done=Observation_Data[5]


            with open(Train_Log_Path, 'a') as TrainLog:
                TrainLog.write(str(action1)+'   '+str(Observation_Data[0])+'   '+str(Observation_Data[1])+'   '+str(Observation_Data[2])+'   '+str(Observation_Data[3])+'   '+str(done)+'\n')
                TrainLog.close()
                              
            exp_pointer += 1
            if exp_pointer == options.MAX_EXPERIENCE:
                exp_pointer = 0

            while os.path.getsize(Action_Path)!=0:   #测时间的时候放后面  
                i=1  

            with open(Action_Path, 'a') as ActionLog:
                ActionLog.write(str(action1)+'   '+str(done))
                ActionLog.close()

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

            end=datetime.datetime.now()
            SpendTime=end-begin
            with open(TrainTime_Log_Path, 'a') as TimeLog:
                TimeLog.write(str(begin)+'   '+str(end)+'   '+str(SpendTime.total_seconds()*1000)+'ms'+'\n')
                TimeLog.close()

        print ("***Episode {} ended with score={},avg_loss={}***".format(i_episode+1, score, sum_loss_value/(local_step+1)))
        with open(Score_Log_Path, 'a') as Sco:
            Sco.write(str(i_episode+1)+'   '+str(score)+'   '+str(sum_loss_value)+'   '+str(local_step)+'   '+str(sum_loss_value/local_step)+'\n')
            Sco.close()
        #saver.save(sess, '../Graph/' + 'dqn')   
        #print("Graph_saved !")

    #saver.save(sess, '../Graph/' + 'dqn')   
    #print("Graph_saved !")
        
    print("Training Finished !")

if __name__ == "__main__":
    TrainOnLineGraph()
