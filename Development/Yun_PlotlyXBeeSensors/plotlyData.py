

import urllib2
import time
import sys
from subprocess import call
import plotly
from platform import system

if system() == 'Linux':
    urlbase = 'http://localhost/arduino/'
else:
    urlbase = 'http://arduino.local/arduino/'


## On Linino, need to reset MCU here.

##sys.stdout.write( 'Getting coordinator...' )
##sys.stdout.flush()
##coordinatorAddress = urllib2.urlopen( urlbase + 'coordinator' ).read()
##print( 'done.' )
##print( 'Coordinator: ' + coordinatorAddress )
##
##
##sys.stdout.write( 'Telling Coordinator to do discovery...' )
##sys.stdout.flush()
##dodisco = urllib2.urlopen( urlbase + 'dodiscovery' )
##time.sleep( 5 )
##print( 'done.' )
##
sys.stdout.write( 'Getting list of nodes...' )
sys.stdout.flush();
nodeString = urllib2.urlopen( urlbase + 'listnodes' ).read()
print( 'done.' )

nodeAddresses = nodeString.split()

sys.stdout.write( 'Initializing plotly...' )
sys.stdout.flush()
py = plotly.plotly("hherhold", "831opc9jsj")
print( 'done.' )

index = 0

while True:
    measUrl = urlbase + 'measure/' + nodeAddresses[ 0 ]
    print( 'Measure url = ' + measUrl );
    temp = urllib2.urlopen( measUrl ).read()
    print( temp )
    py.plot( [ index ], [ float( temp ) ], filename='temp_1trace', fileopt='extend' )
    time.sleep( 10 );
    index = index + 1
