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
    toff = 1
    ton = 5
    board = C_BoardSerialControl()
    
    while True:
        board.lights.switch_off()
        time.sleep(toff)
        board.lights.switch_on_slot1_front_all()
        time.sleep(ton)
        
        board.lights.switch_off()
        time.sleep(toff)
        board.lights.switch_on_slot2_front_all()
        time.sleep(ton)

        board.lights.switch_off()
        time.sleep(toff)
        board.lights.switch_on_slot3_front_all()
        time.sleep(toff)

    # test lights working
    board.lights.switch_off()
 #   time.sleep(toff)
 #   board.lights.switch_on_all()
 #   time.sleep(ton)


    board.lights.switch_off()
#    time.sleep(toff)
#    board.lights.switch_on_slot1_reward()
#    board.lights.switch_on_slot1_front_all()
#    board.lights.switch_on_slot2_reward()
#    board.lights.switch_on_slot3_reward()
#    time.sleep(ton)
    board.lights.switch_off()
    board.lights.switch_on_slot3_front_all()

    #board.lights.switch_off() #switch off all lights
    print '\tdone'
    pass

