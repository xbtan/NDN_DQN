# -*- coding: utf-8 -*-    #兼容中文

import tensorflow as tf
import numpy as np
import gym
import time
import os

Observation_Path='../Data/Train/Observation.txt'
Action_Path='../Data/Train/Action.txt'
Train_Log_Path='../Data/Train_Log/Train_Log.txt'

#####################  hyper parameters  ####################
MAX_EPISODES = 3000
MAX_EP_STEPS = 200
LR_A = 0.001    # learning rate for actor
LR_C = 0.002    # learning rate for critic
GAMMA = 0.9     # reward discount
TAU = 0.01      # soft replacement    软更新tao
MEMORY_CAPACITY = 1000
BATCH_SIZE = 32

ENV_NAME = 'Pendulum-v0'

###############################  DDPG  ####################################
class DDPG(object):
    def __init__(self, a_dim, s_dim, a_bound,):
        self.memory = np.zeros((MEMORY_CAPACITY, s_dim * 2 + a_dim + 1), dtype=np.float32)    #memory 存放的是序列（s,a,r,s+1）= s*2+a+1(r=1)
        self.pointer = 0
        self.sess = tf.Session()

        self.a_dim, self.s_dim, self.a_bound = a_dim, s_dim, a_bound,
        self.S = tf.placeholder(tf.float32, [None, s_dim], 's')
        self.S_ = tf.placeholder(tf.float32, [None, s_dim], 's_')
        self.R = tf.placeholder(tf.float32, [None, 1], 'r')

        #建立网络，actor网络输入是S,critic输入是s,a
        with tf.variable_scope('Actor'):
            self.a = self._build_a(self.S, scope='eval', trainable=True)
            a_ = self._build_a(self.S_, scope='target', trainable=False)
        with tf.variable_scope('Critic'):
            # assign self.a = a in memory when calculating q for td_error,
            # otherwise the self.a is from Actor when updating Actor
            q = self._build_c(self.S, self.a, scope='eval', trainable=True)
            q_ = self._build_c(self.S_, a_, scope='target', trainable=False)

        # networks parameters
        self.ae_params = tf.get_collection(tf.GraphKeys.GLOBAL_VARIABLES, scope='Actor/eval')
        self.at_params = tf.get_collection(tf.GraphKeys.GLOBAL_VARIABLES, scope='Actor/target')
        self.ce_params = tf.get_collection(tf.GraphKeys.GLOBAL_VARIABLES, scope='Critic/eval')
        self.ct_params = tf.get_collection(tf.GraphKeys.GLOBAL_VARIABLES, scope='Critic/target')

        # target net replacement
        self.soft_replace = [[tf.assign(ta, (1 - TAU) * ta + TAU * ea), tf.assign(tc, (1 - TAU) * tc + TAU * ec)]
                             for ta, ea, tc, ec in zip(self.at_params, self.ae_params, self.ct_params, self.ce_params)]

        q_target = self.R + GAMMA * q_
        # in the feed_dic for the td_error, the self.a should change to actions in memory
        td_error = tf.losses.mean_squared_error(labels=q_target, predictions=q)
        self.ctrain = tf.train.AdamOptimizer(LR_C).minimize(td_error, var_list=self.ce_params)

        a_loss = - tf.reduce_mean(q)    # maximize the q
        self.atrain = tf.train.AdamOptimizer(LR_A).minimize(a_loss, var_list=self.ae_params)

        self.sess.run(tf.global_variables_initializer())

    def choose_action(self, s):
        return self.sess.run(self.a, {self.S: s[np.newaxis, :]})[0]

    def learn(self):
        # soft target replacement
        self.sess.run(self.soft_replace)

        indices = np.random.choice(MEMORY_CAPACITY, size=BATCH_SIZE)
        bt = self.memory[indices, :]
        bs = bt[:, :self.s_dim]
        ba = bt[:, self.s_dim: self.s_dim + self.a_dim]
        br = bt[:, -self.s_dim - 1: -self.s_dim]
        bs_ = bt[:, -self.s_dim:]

        self.sess.run(self.atrain, {self.S: bs})
        self.sess.run(self.ctrain, {self.S: bs, self.a: ba, self.R: br, self.S_: bs_})

    def store_transition(self, s, a, r, s_):
        transition = np.hstack((s, a, [r], s_))
        index = self.pointer % MEMORY_CAPACITY  # replace the old memory with new memory
    
        self.memory[index, :] = transition
        self.pointer += 1

    def _build_a(self, s, scope, trainable):
        with tf.variable_scope(scope):
            net = tf.layers.dense(s, 30, activation=tf.nn.relu, name='l1', trainable=trainable)
            a = tf.layers.dense(net, self.a_dim, activation=tf.nn.tanh, name='a', trainable=trainable)
            return tf.multiply(a, self.a_bound, name='scaled_a')

    def _build_c(self, s, a, scope, trainable):
        with tf.variable_scope(scope):
            n_l1 = 30
            w1_s = tf.get_variable('w1_s', [self.s_dim, n_l1], trainable=trainable)
            w1_a = tf.get_variable('w1_a', [self.a_dim, n_l1], trainable=trainable)
            b1 = tf.get_variable('b1', [1, n_l1], trainable=trainable)
            net = tf.nn.relu(tf.matmul(s, w1_s) + tf.matmul(a, w1_a) + b1)
            return tf.layers.dense(net, 1, trainable=trainable)  # Q(s,a)

###############################  training  ####################################
if __name__ == "__main__":
    with open(Observation_Path, 'w') as Observe:   
        Observe.close()
    with open(Action_Path, 'w') as Act:
        Act.close()
    with open(Train_Log_Path, 'w') as Tra:
        Tra.close()

    s_dim = 6
    a_dim = 1

    a_bound=np.array([2.0])    #应该是从哪个边界开始探索（上边界开始还是下边界开始）
    
    ddpg = DDPG(a_dim, s_dim, a_bound)
    
    saver = tf.train.Saver(max_to_keep=0)
    checkpoint = tf.train.get_checkpoint_state("./Graph")
    if checkpoint and checkpoint.model_checkpoint_path:
        saver.restore(sess, checkpoint.model_checkpoint_path)
        print("Successfully loaded:", checkpoint.model_checkpoint_path)
        OldGraph=True
    else:
        print("Could not find old network weights")
        OldGraph=False

    var = 3  # control exploration
    t1 = time.time()
    for i_episede in range(MAX_EPISODES):
        s = np.array([1,1,1,10,0,10])
        ep_reward = 0
        done=0
        a=np.array([1.0])
        Episode_Step=0
        while not done:
            while os.path.getsize(Observation_Path)==0:   
                i=1
            Observation_Data=np.loadtxt(Observation_Path)
            with open(Observation_Path, 'w') as Observe:
                Observe.close()

            s_ = np.array([Observation_Data[0], Observation_Data[1], Observation_Data[2],Observation_Data[3],Observation_Data[4],Observation_Data[5]])
            done=Observation_Data[6]
            Episode_Step+=1
            #if Episode_Step>=200:
            #    done=1
           
            reward1=np.log(Observation_Data[2]/Observation_Data[5])*60
            reward2=Observation_Data[3]*Observation_Data[3]
            reward3=Observation_Data[4]
            reward4=reward1-reward2-reward3
         
            r = reward4
            
            #r=-abs(Observation_Data[0]-80)

            with open(Train_Log_Path, 'a') as TrainLog:
                TrainLog.write('%-4d'%Observation_Data[1]+'%-10f'%reward1+'   '+'%-10f'%reward2+'   '+'%-10f'%reward3+'   '+'%-10f'%reward4+'   '+'%-1d'%done+' '+'%-4d'%(i_episede+1)+'\n')
                TrainLog.close()

            ddpg.store_transition(s, a, r, s_)

            if ddpg.pointer > MEMORY_CAPACITY:
                var *= .9995
                ddpg.learn()

            s = s_
            ep_reward += r

            a = ddpg.choose_action(s)
            
            #a1=
            #print "a1:"+str(a)
            #b = np.array([1.0])
            #c = np.array([100.0])
            #a = tf.add(tf.multiply(tf.add(a,b),c),b)
            #print "a1:"+str(a)
            #a=a*50+101
            #print "a2:"+str(a)
            
            a = np.clip(np.random.normal(a, var), -2, 2)
            m_cwnd=(a[0]+2)*50+1
            #print ('cwnd:',m_cwnd)
            
            #print "a2:"+str(a)
            #print np.shape(a)
            #print a
            #print a[0]
            with open(Action_Path, 'a') as ActionLog:
                ActionLog.write(str(m_cwnd)+'   '+str(done))
                ActionLog.close()

        print('Episode:', i_episede, ' Reward: %i' % int(ep_reward), 'Explore: %.2f' % var, )
        if (i_episode+1)%2==0:
            os.makedirs('./Graph/'+str(i_episode+1))
            saver.save(sess, './Graph/' + str(i_episode+1) + '/' + 'dqn',global_step = i_episode+1)   
            print("Graph_saved !")
    print('Running time: ', time.time() - t1)
