import socket
import time
UDP_IP = "192.168.100.1"
UDP_PORT = 8765
sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

n=0
M0=bytearray(1472)
t0=time.time();
for i in range(10,1400):
    M0[i]=i%255

while True:

    #time.sleep(0.000001)
    n=n+1
    M0[:4] = n.to_bytes(4, 'big')
    if (n%100000 == 1 and n>1):
        time.sleep(1 /1000000.0)
        t=time.time()
        MB=n*1472/1024/1024.
        dt=(t-t0)
        MBS=MB/dt
        print(f"Event {n} MegaBytes {MB:.2f} Debit Mbit/s {MBS*8:.2f}")
    #        v=input()
    #print("UDP target IP:", UDP_IP)
    #print("UDP target port:", UDP_PORT)
    #print("message:", MESSAGE)
    rs=sock.sendto(M0, (UDP_IP, UDP_PORT))
    #print(n,rs)
