#!/usr/bin/python
import sys    
import time
import string
import random

def rand_generator(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for x in range(size))

sys.path.insert(0, '/usr/lib/python2.7/bridge/') 
from bridgeclient import BridgeClient as bridgeclient
bc = bridgeclient()                              

from subprocess import call

f=open('test_result.txt','w')

cnt4=0
msg_id='a'
n_loops=200

for msg_length in range(80,10,-5):
   print "msg_length set to ",msg_length
   #reset mcu
   print "resetting mcu now"
   call(["reset-mcu"])
   time.sleep(5)
   for loop in range(1,n_loops):
       time.sleep(2)
       if cnt4==0:
         msg_id='a'
         cnt4=1
       elif cnt4==1:
         msg_id='b'
         cnt4=2
       elif cnt4==2:
         msg_id='c'
         cnt4=3
       else:
         msg_id='d'
         cnt4=0
       msg=rand_generator(msg_length)  
       toput=msg_id+msg
       print "put",toput
       bc.put('key',toput)
      
       # wait for identical returned value
       trial=1
       while trial<4:
           time.sleep(1)
           r = bc.get('key2')
           print "get ",r
           if r is None:
             print "   No answer key found yet"
           else:
             if r==msg:
                 print "   OK after",trial," trials"
                 break
             else:
                 print "   FAIL"
                 trial=trial+1
       if trial>=4:
         f.write("msg_length=")
         f.write(str(msg_length))
         f.write(" ")
         f.write("FAILURE after ")
         f.write(str(loop))
         f.write(" messages\n")
         break
       elif loop==n_loops-1:
         f.write("msg_length=")
         f.write(str(msg_length))
         f.write(" ")
         f.write("SUCCES ")
         f.write(str(loop))
         f.write(" messages\n")
f.close()
