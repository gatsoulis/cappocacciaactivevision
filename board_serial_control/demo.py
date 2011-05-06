#!/usr/bin/env python

'''
	Author: Yiannis Gatsoulis (i.gatsoulis at ulster dot ac dot uk)
	Filename: demo.py
	Date: 02/05/2011
	Licence: GPL. Basically feel free to modify it. It would be nice if you drop me an email.
	Description: Simple demo for the lights. See board_serial_control.py for extra info.
'''


import time
from board_serial_control import C_BoardSerialControl

ON = 1
OFF = 0

if __name__ == '__main__':
    delay = 1
    board = C_BoardSerialControl()

    print 'Starting lights demo'

    # test lights working
    board.lights.switch_off()
    time.sleep(delay)
    board.lights.switch_on_all()
    time.sleep(delay)

    board.lights.switch_on(s1bt=ON) #switch on slot1 base top light
    time.sleep(delay)
    board.lights.switch_off() #switch off all lights
    time.sleep(delay)
    board.lights.switch_on(s1bt=ON, s2fr=ON) #switch on slot1 base top light and slot2 front right light
    time.sleep(delay)
    board.lights.switch_off() #switch off all lights
    time.sleep(delay)
    board.lights.switch_on(s1bt=ON, s2fr=ON) #switch on slot1 base top light and slot2 front right light
    time.sleep(delay)
    board.lights.switch_off(True) #switch off all lights but keep state
    time.sleep(delay)
    board.lights.switch_on() #switch on lights from previous state
    time.sleep(delay)
    board.lights.switch_off(True) #switch off all lights but keep state
    time.sleep(delay)
    board.lights.switch_on(s3w=ON) #switch on lights from previous state and also slot3 reward light
    time.sleep(delay)

    board.lights.switch_off() #switch off all lights
    time.sleep(delay)
    board.lights.switch_on_slot1_base_all() #switch on slot1 base lights
    time.sleep(delay)
    board.lights.switch_on_slot1_front_all() #switch on slot1 front lights
    time.sleep(delay)
    board.lights.switch_on_slot1_reward() #switch on slot1 reward light
    time.sleep(delay)
    
    board.lights.switch_off() #switch off all lights
    time.sleep(delay)
    board.lights.switch_on_slot2_base_all() #switch on slot2 base lights
    time.sleep(delay)
    board.lights.switch_on_slot2_front_all() #switch on slot2 front lights
    time.sleep(delay)
    board.lights.switch_on_slot2_reward() #switch on slot2 reward light
    time.sleep(delay)

    board.lights.switch_off() #switch off all lights
    time.sleep(delay)
    board.lights.switch_on_slot3_base_all() #switch on slot3 base lights
    time.sleep(delay)
    board.lights.switch_on_slot3_front_all() #switch on slot3 front lights
    time.sleep(delay)
    board.lights.switch_on_slot3_reward() #switch on slot3 reward light
    time.sleep(delay)

    board.lights.switch_off() #switch off all lights
    print '\tdone'
    pass

