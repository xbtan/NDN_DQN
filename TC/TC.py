# -*- coding:utf-8 -*-

import numpy as np
import os
import time

TCDataPath='TN.txt'

if __name__=="__main__":
    os.system('sudo tcdel --device enp9s0 --all')
    TCData = np.loadtxt(TCDataPath)
    count=0
    while count<=47:    #229
        Temp=int(TCData[count])
        #os.system("ls")
        os.system("sudo tcset --device enp9s0 --rate "+str(Temp)+"M --delay 10ms --loss 0.02 --network 192.168.4.9 --overwrite")
        print str(count)+":"+str(Temp)
        count+=1
        time.sleep(1)    #0.001 
    
    print "************************************************"
    print "************************************************"
    print "************************************************"
    

