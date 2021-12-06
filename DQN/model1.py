# -*- coding: utf-8 -*-    #兼容中文

import sys

if not hasattr(sys, 'argv'):
    sys.argv  = ['']

from argparse import ArgumentParser

def get_options():
    parser = ArgumentParser()
    parser.add_argument('--MAX_EPISODE', type=int, default=5000,   #2000
                        help='max number of episodes iteration')
    parser.add_argument('--ACTION_DIM', type=int, default=7,
                        help='number of actions one can take')
    parser.add_argument('--OBSERVATION_DIM', type=int, default=5,    #4
                        help='number of observations one can see')
    parser.add_argument('--GAMMA', type=float, default=0.9,    #0.9
                        help='discount factor of Q learning')
    parser.add_argument('--INIT_EPS', type=float, default=1.0,    #1.0
                        help='initial probability for randomly sampling action')
    parser.add_argument('--FINAL_EPS', type=float, default=1e-5,    #1e-5
                        help='finial probability for randomly sampling action')
    parser.add_argument('--EPS_DECAY', type=float, default=0.97,    #0.95
                        help='epsilon decay rate')
    parser.add_argument('--EPS_ANNEAL_STEPS', type=int, default=20,   #10
                        help='steps interval to decay epsilon')
    parser.add_argument('--LR', type=float, default=1e-3,    #1e-3
                        help='learning rate')
    parser.add_argument('--MAX_EXPERIENCE', type=int, default=200,    #200  
                        help='size of experience replay memory')
    parser.add_argument('--BATCH_SIZE', type=int, default=200,   #256
                        help='mini batch size'),
    parser.add_argument('--H1_SIZE', type=int, default=256,    #128
                        help='size of hidden layer 1')
    parser.add_argument('--H2_SIZE', type=int, default=128,    #128
                        help='size of hidden layer 2')
    parser.add_argument('--H3_SIZE', type=int, default=256,    #128
                        help='size of hidden layer 3')
    parser.add_argument('--H4_SIZE', type=int, default=128,    #128
                        help='size of hidden layer 4')
    parser.add_argument('--H5_SIZE', type=int, default=64,    #128
                        help='size of hidden layer 5')
    options = parser.parse_args()
    return options
