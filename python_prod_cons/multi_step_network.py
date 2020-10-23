import concurrent.futures
import time
import random
import threading

END = object()

sample_file = open('sample.txt','r')
buff=[]

class Pipeline:
  
   def __init__(self):
     self.message = 0
     self.producer_lock = threading.Lock()
     self.consumer_lock = threading.Lock()
     self.consumer_lock.acquire()
  
   def get_message(self, name):
     self.consumer_lock.acquire()
     message = self.message
     self.producer_lock.release()
     return message
  
   def set_message(self, message, name):
     self.producer_lock.acquire()
     self.message = message
     self.consumer_lock.release() 

def prod(pl):
    tmp_count=0
    while True:
        tmp_count+=1
        time.sleep(random.random()) # 간헐적으로 들어오는 라인들
        line = sample_file.readline()
        if not line: break
        pl.set_message(line,"prod")
    
    pl.set_message(END,"prod")

def cons(pl):
    req = 0
    while req is not END:
        req = pl.get_message("cons")
        if req is not END:
            time.sleep(random.random())
            buff.append(req)
            print('cons store data : ',req)
    buff.append(END)

def prod2(pl):

    while True:
        if buff!=[]:
          time.sleep(0.001)# 간헐적으로 들어오는 라인들
          line = buff.pop(0)
          if line == END: break
          pl.set_message(line,"buff_prod")
    
    pl.set_message(END,"buff_prod")

def cons2(pl):
    req = 0
    while req is not END:
        req = pl.get_message("buff_cons")
        if req is not END:
            time.sleep(random.random())

            print('database store data : ',req)
st = time.time()
pl = Pipeline()
pl2= Pipeline()
with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
    executor.submit(prod,pl)
    executor.submit(cons,pl)
    executor.submit(prod2,pl2)
    executor.submit(cons2,pl2)
end=time.time()
print('time: ',end-st)
