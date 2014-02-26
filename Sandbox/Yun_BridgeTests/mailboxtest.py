

######### Feb 18 2014
######### Mailbox not working - known issue and logged in arduino bug db

#!/usr/bin/python
import sys    
import time
import string

sys.path.insert(0, '/usr/lib/python2.7/bridge/') 

import mailbox

from subprocess import call
print "resetting mcu now"
call(["reset-mcu"])
time.sleep(5)

mailbox.writeMessage( "blah" )


