#!/usr/bin/env python

'''
	Author: Yiannis Gatsoulis (i.gatsoulis at ulster dot ac dot uk)
	Filename: board_serial_control.py
	Date: 02/05/2011
	Licence: GPL. Basically feel free to modify it. It would be nice if you drop me an email.
	Description: Able to control the lights of the ImClever experimental board from the serial board.
	Instructions: Plug out the corresponding cable from the usb hub underneath the board, and plug it in directly to your computer. For the usb2serial converter you might need the ftdi driver (http://www.ftdichip.com/FTDrivers.htm). However, on my machine running Ubuntu 10.04 (kernel:2.6.32-31-generic) it was already integrated in the kernel. If the wiring on your board is different than the programs change the <self.mapping> dictionary. Also the activation codes might be different. Search for self.status[m['d6']] = 1 and self.status[m['d7']] = 0 and change them accordingly. Any questions feel free to email me.
	Note: The program is based on a different wiring to the original schematic based on the board I had available at the time. Change self.mapping if you have a different wiring.
	Thanks: Domenico Formica for providing hardware information, the schematic and recommendation to bypass the usb-hub
	Todo: It shouldn't be very difficult to add the motors () and other control, by creating a new class for the motor in a similar manner and extending the C_BoardSerialControl class. Regarding the motors, from Domenico: "It is a commercial controller sold by Parallax, you can find all the information you need at the following link:
	http://www.parallax.com/StoreSearchResults/tabid/768/List/0/SortField/4/ProductID/595/Default.aspx?txtSearch=usb+controller
	The three motors of the board are plugged in the P1, P2, P3 slots of the controller."

'''


import sys
import serial
import math

ON = 1
OFF = 0
KEEP = -1

LIGHTS_PORT = '/dev/ttyUSB0'
LIGHTS_BAUDRATE = 9600


def arraybyte_to_char(a):
    if len(a) != 8:
        print "Error (arraybute_to_char): Parameter <a> should be an array of length 8"
        sys.exit(1)
    dec = int(0)
    for i in range(len(a)):
        dec += int(math.pow(2,i)) * a[i]
    return chr(dec)
    pass


##########
class C_LightsSerialControl:
    def __init__(self, port=LIGHTS_PORT, baudrate=LIGHTS_BAUDRATE):
        try:
            print 'Opening lights port:', LIGHTS_PORT, ' at baudrate:', LIGHTS_BAUDRATE
            self.ser = serial.Serial(port, baudrate, timeout=3)
            print '\tsuccess'
        except:
            print 'Failed to open serial port. Make sure the port name is correct and you have root rights.'
            sys.exit(1)
        '''
            Change the following dictionary depending on your wiring
            See the schematic for details.
            Naming scheme is
                s1br:slot1_base_right (as you look at it)
                s1ft:slot1_front_top (as you look at it)
                s1w: slot1_reward
        '''        
        self.mapping = {'s1br':0, 's1bt':1, 's1bl':2, 's1fr':3, 's1ft':4, 's1fl':5, 's1w':6, 's2w':7, 's2br':8, 's2bt':9, 's2bl':10, 's2fr':11, 's2ft':12, 's2fl':13, 's3w':14, 'notused':15, 's3br':16, 's3bt':17, 's3bl':18, 's3fr':19, 's3ft':20, 's3fl':21, 'd6':22, 'd7':23}
        self.status = [OFF]*24
        pass
    
    def __del__(self):
    	self.close()
        
    def close(self):
        print 'Closing lights port:', LIGHTS_PORT
        try:
            self.ser.close()
            print '\tsuccess'
        except:
            print 'failed, maybe you do not have the rights to close this port'
            sys.exit(1)
    
    # default, all KEEP. control code set lights.
    def switch_on(self, s1bt=KEEP, s1br=KEEP, s1bl=KEEP, s1ft=KEEP, s1fr=KEEP, s1fl=KEEP, s1w=KEEP, s2w=KEEP, s2bt=KEEP, s2br=KEEP, s2bl=KEEP, s2ft=KEEP, s2fr=KEEP, s2fl=KEEP, s3w=KEEP, s3bt=KEEP, s3br=KEEP, s3bl=KEEP, s3ft=KEEP, s3fr=KEEP, s3fl=KEEP):
        m = self.mapping
        #print 'Switching on lights'
        self.status[m['d6']] = 1
        self.status[m['d7']] = 0
        
        if s1bt!=KEEP: self.status[m['s1bt']] = s1bt
        if s1br!=KEEP: self.status[m['s1br']] = s1br
        if s1bl!=KEEP: self.status[m['s1bl']] = s1bl 
        if s1ft!=KEEP: self.status[m['s1ft']] = s1ft 
        if s1fr!=KEEP: self.status[m['s1fr']] = s1fr 
        if s1fl!=KEEP: self.status[m['s1fl']] = s1fl
        if s1w!=KEEP: self.status[m['s1w']] = s1w
        
        if s2w!=KEEP: self.status[m['s2w']] = s2w
        if s2bt!=KEEP: self.status[m['s2bt']] = s2bt
        if s2br!=KEEP: self.status[m['s2br']] = s2br
        if s2bl!=KEEP: self.status[m['s2bl']] = s2bl
        if s2ft!=KEEP: self.status[m['s2ft']] = s2ft
        if s2fr!=KEEP: self.status[m['s2fr']] = s2fr
        if s2fl!=KEEP: self.status[m['s2fl']] = s2fl

        if s3w!=KEEP: self.status[m['s3w']] = s3w
        if s3bt!=KEEP: self.status[m['s3bt']] = s3bt
        if s3br!=KEEP: self.status[m['s3br']] = s3br
        if s3bl!=KEEP: self.status[m['s3bl']] = s3bl
        if s3ft!=KEEP: self.status[m['s3ft']] = s3ft
        if s3fr!=KEEP: self.status[m['s3fr']] = s3fr
        if s3fl!=KEEP: self.status[m['s3fl']] = s3fl

        # and now write to the port the self.status which is a 3byte thing, simple!
        #print self.status
        b = arraybyte_to_char(self.status[0:8])
        c = arraybyte_to_char(self.status[8:16])
        d = arraybyte_to_char(self.status[16:24])
        msg = b+c+d
        self.ser.write(msg)
        pass


    def switch_off(self, keepstate=False):
        m = self.mapping
        if keepstate:
            self.status[m['d6']] = 0
            self.status[m['d7']] = 0
        else:
            self.status = [OFF]*24
        b = arraybyte_to_char(self.status[0:8])
        c = arraybyte_to_char(self.status[8:16])
        d = arraybyte_to_char(self.status[16:24])
        msg = b+c+d
        self.ser.write(msg)
        pass

    #convenience functions       
    def switch_on_all(self):
        self.switch_on(s1bt=ON, s1br=ON, s1bl=ON, s1ft=ON, s1fr=ON, s1fl=ON, s1w=ON, s2w=ON, s2bt=ON, s2br=ON, s2bl=ON, s2ft=ON, s2fr=ON, s2fl=ON, s3w=ON, s3bt=ON, s3br=ON, s3bl=ON, s3ft=ON, s3fr=ON, s3fl=ON)
    
    def switch_on_slot1_base_all(self):
        self.switch_on(s1bt=ON, s1br=ON, s1bl=ON)

    def switch_on_slot1_front_all(self):
        self.switch_on(s1ft=ON, s1fr=ON, s1fl=ON)

    def switch_on_slot2_base_all(self):
        self.switch_on(s2bt=ON, s2br=ON, s2bl=ON)

    def switch_on_slot2_front_all(self):
        self.switch_on(s2ft=ON, s2fr=ON, s2fl=ON)

    def switch_on_slot3_base_all(self):
        self.switch_on(s3bt=ON, s3br=ON, s3bl=ON)

    def switch_on_slot3_front_all(self):
        self.switch_on(s3ft=ON, s3fr=ON, s3fl=ON)

    def switch_on_slot1_reward(self):
        self.switch_on(s1w=ON)

    def switch_on_slot2_reward(self):
        self.switch_on(s2w=ON)

    def switch_on_slot3_reward(self):
        self.switch_on(s3w=ON)


##########
class C_BoardSerialControl:
    def __init__(self):
        self.lights = C_LightsSerialControl()
        pass

