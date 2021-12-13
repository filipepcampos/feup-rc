import os
import subprocess
import time
import sys

IS_EMITTER = False

def init():
    if(len(sys.argv) != 2):
        exit(-1)
    global IS_EMITTER
    IS_EMITTER = sys.argv[1] == 'e'
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
    

def run_emitter(i):
    with open(f'./logs/log{i}', 'w+') as f:
        input("")
        start = time.time()
        emitter = subprocess.Popen(['./build/application', 'emitter', '0', './files/pinguim.gif'], stdout=f)
    emitter.wait()
    return time.time() - start

def run_receiver(i):
    with open(f'./logs/log{i}', 'w+') as f:
        input("")
        start = time.time()
        receiver = subprocess.Popen(['./build/application', 'receiver', '0', './build/output'], stdout=f)
    receiver.wait()
    total = time.time() - start
    os.system("diff ./files/pinguim.gif ./build/output")
    return total

FER = [0, 1, 5, 10]
TPROP = [0, 200000, 1000000]
INFOSIZE = [256, 512]

def main():
    init()
    i = 0
    for tprop in TPROP:
        for fer in FER:
            for isize in INFOSIZE:
                clean()
                make(fer,tprop,isize)
                print(f'[{i}] {tprop},{fer/100},{isize}: ', end='', flush=True)
                print(run(i))
                i += 1

if __name__ == "__main__":
    main()
