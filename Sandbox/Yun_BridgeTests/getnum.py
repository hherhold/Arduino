#!/usr/bin/python
import sys    
import time
import string

sys.path.insert(0, '/usr/lib/python2.7/bridge/') 
from bridgeclient import BridgeClient as bridgeclient
bc = bridgeclient()                              

from subprocess import call


#reset mcu
print "resetting mcu now"
call(["reset-mcu"])
time.sleep(5)

toput = "

print "put",toput
bc.put('',toput)
      

###

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

