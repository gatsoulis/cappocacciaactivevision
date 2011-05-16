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

if __name__ == '__main__':
    t = 0.5
    board = C_BoardSerialControl()

    # test lights working
    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_all()
    time.sleep(t)

    # switch on section by section   
    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot1_base_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot1_front_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot1_reward()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot2_base_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot2_front_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot2_reward()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot3_base_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot3_front_all()
    time.sleep(t)

    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot3_reward()
    time.sleep(t)

    #incremental, keeping state    
    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_slot1_base_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot1_front_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot1_reward()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot2_base_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot2_front_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot2_reward()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot3_base_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot3_front_all()
    time.sleep(t)

    board.lights.switch_off(True)
    time.sleep(t)
    board.lights.switch_on_slot3_reward()
    time.sleep(t)

    # test complete
    board.lights.switch_off()
    time.sleep(t)
    board.lights.switch_on_all()
    time.sleep(t)
    
    #switch off lights and exit
    board.lights.switch_off()
    time.sleep(t)
    pass

