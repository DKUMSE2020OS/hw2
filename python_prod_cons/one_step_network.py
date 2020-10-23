import concurrent.futures
import time
import random
import threading

END = object()

sample_file = open('sample.txt','r')

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

    while True:
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
            print('cons store data : ',req)

pl = Pipeline()
with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
    executor.submit(prod,pl)
    executor.submit(cons,pl)
        