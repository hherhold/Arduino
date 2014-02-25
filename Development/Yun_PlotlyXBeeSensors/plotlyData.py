

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
    
    # Get the list of measurements
    tempMeasurements = []
    for node in nodeAddresses:
        measUrl = urlbase + 'measure/' + node
        print( 'Measure url = ' + measUrl );
        temp = urllib2.urlopen( measUrl ).read()
        tempMeasurements = tempMeasurements + [float(temp)]

    print( 'Measurements: ' + str(tempMeasurements) )

    try:
        if len( tempMeasurements ) == len( nodeAddresses ):
            trace = 0
            for curTemp in tempMeasurements:
                if index == 0:
                    print( 'append index ' + str(index) + ' temp ' + str(curTemp) )
                    py.plot( [ index ], [ curTemp ], 
                             filename='temp_multitrace', fileopt='append' )
                else:
                    print( 'extend index ' + str(index) + ' temp ' + 
                           str(curTemp) + ' trace ' + str(trace) )
                    py.plot( [ index ], [ curTemp ], 
                             filename='temp_multitrace', fileopt='extend', traces=[trace] )
                    trace = trace + 1
            index = index + 1

    except:
        print "Unexpected error:", sys.exc_info()[0]
        pass


    time.sleep( 10 );
        
###    [index], [temp], [index], [temp]
###
