import os
import subprocess
import time
import sys
import socket

HOST = '192.168.1.14'
PORT = 9001

IS_EMITTER = False
s = None
conn = None

def init():
    if(len(sys.argv) != 2):
        exit(-1)
    global IS_EMITTER, conn, s
    IS_EMITTER = sys.argv[1] == 'e'

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if IS_EMITTER:
        s.connect((HOST, PORT))
        print("Connected")
    else:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        print("Connected")

    os.system("rm -rf ./logs; mkdir -p logs")

def clean():
    os.system(f'make clean > /dev/null')

def make(fer,tprop,infosize):
    os.system(f'make FER={fer} T_PROP={tprop} MAX_INFO_SIZE={infosize} > /dev/null')

def run(i): 
    if IS_EMITTER:
        return run_emitter(i)
    else:
        return run_receiver(i)
    
def sync():
    if IS_EMITTER:
        data = s.recv(1024)
        s.send(data)
        time.sleep(0.005)
    else:
        conn.send(b'foo')
        data = conn.recv(1024)

def run_emitter(i):
    with open(f'./logs/log{i}', 'w+') as f:
        sync()
        start = time.time()
        emitter = subprocess.Popen(['./build/application', 'emitter', '0', './files/pinguim.gif'], stdout=f)
    emitter.wait()
    total = time.time() - start
    sync()
    return total

def run_receiver(i):
    with open(f'./logs/log{i}', 'w+') as f:
        sync()
        start = time.time()
        receiver = subprocess.Popen(['./build/application', 'receiver', '0', f'./build/output{i}'], stdout=f)
    receiver.wait()
    total = time.time() - start
    os.system(f"diff ./files/pinguim.gif ./build/output{i}")
    sync()
    return total

FER = [0, 1, 5, 10]
TPROP = [0, 200000, 1000000]
INFOSIZE = [256, 512, 1024, 2048]

def main():
    init()
    i = 0
    for tprop in TPROP:
        for fer in FER:
            for isize in INFOSIZE:
                make(fer,tprop,isize)
                print(f'[{i}] {tprop},{fer/100},{isize}: ', end='', flush=True)
                print(run(i))
                i += 1
    clean()
    conn.close()

if __name__ == "__main__":
    main()
