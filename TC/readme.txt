Build:

g++ TrafficControl.cpp -std=c++11 -o TC
g++ random.cpp -std=c++11 -o random

Run:
./random
./TC 192.168.4.9 TrafficSetData.txt

Init:
sudo tcdel --device enp7s0 --all



#sudo tcset --device enp7s0(local) --rate 250k(bit) --network 192.168.4.9(remote)
sudo tcset --device enp9s0 --delay 3(ms) --network 192.168.2.9
sudo tcdel --device enp7s0 --all
iperf -s
iperf -c 192.168.4.8(remote)
