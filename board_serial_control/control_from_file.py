#!/usr/bin/env python

'''
	Author: Yiannis Gatsoulis (i.gatsoulis at ulster dot ac dot uk)
	Filename: demo.py
	Date: 02/05/2011
	Licence: GPL. Basically feel free to modify it. It would be nice if you drop me an email.
	Description: Simple demo for the lights. See board_serial_control.py for extra info.
'''

import sys
import time
from board_serial_control import C_BoardSerialControl

ON = 1
OFF = 0

if __name__ == '__main__':
    t = 0.2
    board = C_BoardSerialControl()
    fname = 'state.txt'

    while True:
        time.sleep(t)
        try:
            f = open(fname)
            lines = f.readlines()
            f.close()
            bcd = []
            for line in lines:
                if not line[0] == '#':
                    line = line.strip()
                    line = line.split()
                    line.reverse()
                    for i in line:
                        bcd.append(int(i))
            board.lights.switch_on(s1bt=bcd[0], s1br=bcd[1], s1bl=bcd[2], s1ft=bcd[3], s1fr=bcd[4], s1fl=bcd[5], s1w=bcd[6], s2w=bcd[7], s2bt=bcd[8], s2br=bcd[9], s2bl=bcd[10], s2ft=bcd[11], s2fr=bcd[12], s2fl=bcd[13], s3w=bcd[14], s3bt=bcd[16], s3br=bcd[17], s3bl=bcd[18], s3ft=bcd[19], s3fr=bcd[20], s3fl=bcd[21])

        except (KeyboardInterrupt, SystemExit):
            print 'Bye'
            if not f.closed:
                f.close()
            sys.exit(0)
        except:
            print 'file might be opened or you have the wrong filename'
    pass

