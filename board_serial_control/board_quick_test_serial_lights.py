#!/usr/bin/env python

'''
	Author: Yiannis Gatsoulis (i.gatsoulis at ulster dot ac dot uk)
	Filename: board_quick_test_serial_lights.py
	Date: 02/05/2011
	Licence: GPL. Basically feel free to modify it. It would be nice if you drop me an email.
	Description: Quick test to send signals to the board for controlling the lights in this case. See board_serial_control.py for extra info.
'''

import sys
import serial

LIGHTS_PORT = '/dev/ttyUSB0'
LIGHTS_BAUDRATE = 9600
LIGHTS_TIMEOUT = 3

print 'Hi'
print 'Trying to open: ', LIGHTS_PORT
try:
    ser = serial.Serial(LIGHTS_PORT, LIGHTS_BAUDRATE, timeout=LIGHTS_TIMEOUT)
    print 'Opened: ', ser.portstr
except:
    print 'Failed to open serial port. Make sure the port name is correct and you have root rights.'
    sys.exit(1)

b = chr(0)
c = chr(0) # 128 does nothing, corresponds to C7
d = chr(64+0) # 64:D6D7=10
msg = b+c+d
ser.write(msg)

print 'Bye'
