# can-client-linux

ip link ls
sudo ip link set up can0 type can bitrate 500000

Test connection:
candump can0

gcc can_client.c -o can_client

Test program:
./can_client
