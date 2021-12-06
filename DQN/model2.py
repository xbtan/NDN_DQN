# -*- coding: utf-8 -*-    #兼容中文

import tensorflow as tf
import numpy as np
import random

class QAgent:
    #定义网络
    def __init__(self, options):
        self.W1 = self.weight_variable([options.OBSERVATION_DIM, options.H1_SIZE])
        self.b1 = self.bias_variable([options.H1_SIZE])
        self.W2 = self.weight_variable([options.H1_SIZE, options.H2_SIZE])
        self.b2 = self.bias_variable([options.H2_SIZE])
        self.W3 = self.weight_variable([options.H2_SIZE, options.H3_SIZE])
        self.b3 = self.bias_variable([options.H3_SIZE])
        self.W4 = self.weight_variable([options.H3_SIZE, options.H4_SIZE])
        self.b4 = self.bias_variable([options.H4_SIZE])  
        self.W5 = self.weight_variable([options.H4_SIZE, options.H5_SIZE])
        self.b5 = self.bias_variable([options.H5_SIZE])               
        self.W6 = self.weight_variable([options.H5_SIZE, options.ACTION_DIM])
        self.b6 = self.bias_variable([options.ACTION_DIM])
    
    # Weights initializer
    def xavier_initializer(self, shape):
        dim_sum = np.sum(shape)
        if len(shape) == 1:
            dim_sum += 1
        bound = np.sqrt(6.0 / dim_sum)
        return tf.random_uniform(shape, minval=-bound, maxval=bound)

    # Tool function to create weight variables
    def weight_variable(self, shape):
        return tf.Variable(self.xavier_initializer(shape))

    # Tool function to create bias variables
    def bias_variable(self, shape):
        return tf.Variable(self.xavier_initializer(shape))

    def add_value_net(self, options):
        observation = tf.placeholder(tf.float32, [None, options.OBSERVATION_DIM])
        h1 = tf.nn.relu(tf.matmul(observation, self.W1) + self.b1)
        h2 = tf.nn.relu(tf.matmul(h1, self.W2) + self.b2)
        h3 = tf.nn.relu(tf.matmul(h2, self.W3) + self.b3)
        h4 = tf.nn.relu(tf.matmul(h3, self.W4) + self.b4)   #激活
        h5 = tf.nn.relu(tf.matmul(h4, self.W5) + self.b5)   #
        Q = tf.squeeze(tf.matmul(h5, self.W6) + self.b6)   #捏住
        return observation, Q

    def sample_action(self, Q, feed, eps, options):
        act_values = Q.eval(feed_dict=feed)   #action是6维的，算6个的，再取最大
        #print act_values
        randomNum=random.random()
        if randomNum <= eps:
            print (str(randomNum)+"    "+str(eps))
            action_index = random.randrange(options.ACTION_DIM)
        else:
            action_index = np.argmax(act_values)   #np.argmax()函数可以取出“act_values[0]”中的最大值
        action = np.zeros(options.ACTION_DIM)
        #print action_index
        action[action_index] = 1
        return action

    def sample_action2(self, Q, feed, eps, options):
        act_values = Q.eval(feed_dict=feed)   #action是6维的，算6个的，再取最大
        action_index = np.argmax(act_values)   #np.argmax()函数可以取出“act_values[0]”中的最大值
        action = np.zeros(options.ACTION_DIM)
        #print action_index
        action[action_index] = 1
        return action
