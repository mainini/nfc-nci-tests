#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2015 Pascal Mainini
# Licensed under MIT license, see included file LICENSE or
# http://opensource.org/licenses/MIT
#
# Experimental dissector for I²C-traffic from/to a PN7120 NFC-controller.
# Needs I²C-data as bytes in the form 0xNN0xNN...0xNN - all other, non hex values
# are ignored. Data can be given as parameter, via stdin or read from specified file.
#

from __future__ import print_function
import sys
import argparse
import re

class NCIPacket():
    """ Representation of a NCI packet with (yet) unknown type, only the first header byte is dissected here."""
   
    def __init__(self, data):
        self.PKT_DATA = 0
        self.PKT_CTRL_CMD = 1    # Control packet, command as payload
        self.PKT_CTRL_RES = 2    # Control packet, response as payload
        self.PKT_CTRL_NOT = 3    # Control packet, notification message as payload

        self.byte0 = int(data[0], 16)
        self.MT = self.byte0 >> 5
        self.PBF = (self.byte0 & 16) >> 4

    def __str__(self):
        if self.MT == self.PKT_DATA: 
            cmd = 'DATA'
        elif self.MT == self.PKT_CTRL_CMD:
            cmd = 'CTRL_COMMAND'
        elif self.MT == self.PKT_CTRL_RES:
            cmd = 'CTRL_RESPONSE'
        elif self.MT == self.PKT_CTRL_NOT:
            cmd = 'CTRL_NOTIFICATION'
        else:
            cmd = 'UNKNOWN'

        return '[NCI][MT:{:03b} ({:s}) PBF:{:b}]'.format(self.MT, cmd, self.PBF)

class NCIDataPacket(NCIPacket):
    """ Representation of a NCI data packet, diissects full header of 3 bytes """

    def __init__(self, data):
        NCIPacket.__init__(self, data)
        self.byte1 = int(data[1], 16)
        self.byte2 = int(data[2], 16)

        self.lenPayload = self.byte2
        self.ConnID = (self.byte0 & 15)

    def __str__(self):
        if self.lenPayload > 0:
            payload = '[PAYLOAD (len {:d}): {:s}]'.format(self.lenPayload, str(self.payload))
        else:
            payload = ''

        return '[DATA][PBF:{:b} ConnID:{:04b}]{:s}'.format(self.PBF, self.ConnID, payload)

class NCIControlPacket(NCIPacket):
    """ Representation of a NCI control packet, diissects full header of 3 bytes """

    def __init__(self, data):
        NCIPacket.__init__(self, data)
        self.byte1 = int(data[1], 16)
        self.byte2 = int(data[2], 16)

        self.lenPayload = self.byte2
        self.GID = (self.byte0 & 15)
        self.OID = (self.byte1 & 63)

    def __str__(self):
        if self.lenPayload > 0:
            payload = '[PAYLOAD (len {:d}): {:s}]'.format(self.lenPayload, str(self.payload))
        else:
            payload = ''

        if self.MT == self.PKT_CTRL_CMD:
            return '[COMMAND][PBF:{:b} GID:{:04b} OID:{:06b}]{:s}'.format(self.PBF, self.GID, self.OID, payload)
        elif self.MT == self.PKT_CTRL_RES:
            return '[RESPONSE][PBF:{:b} GID:{:04b} OID:{:06b}]{:s}'.format(self.PBF, self.GID, self.OID, payload)
        elif self.MT == self.PKT_CTRL_NOT:
            return '[NOTIFICATION][PBF:{:b} GID:{:04b} OID:{:06b}]{:s}'.format(self.PBF, self.GID, self.OID, payload)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="""Experimental dissector for I²C-traffic from/to a PN7120 NFC-controller.
Needs I²C-data as bytes in the form 0xNN0xNN...0xNN - all other, non hex values
are ignored. Data can be given as parameter, via stdin or read from specified file.""")
    parser.add_argument('bytes', nargs='?', help='Bytes of I²C-data - if given, file and stdin are ignored')
    parser.add_argument('-f','--file', help='A file containing bytes of I²C-data')
    args = parser.parse_args()

    data = ''
    if args.file:
        f = open(args.file, 'r')
        data = f.read()
        f.close()
    elif args.bytes:
        data = args.bytes
    else:
        data =  sys.stdin.read()

    data = re.findall(r'0x([0-9A-Fa-f][0-9A-Fa-f])*', data)

    while len(data) > 0:
        header0 = data.pop(0)
        pkt = NCIPacket([header0])

        if pkt.MT == pkt.PKT_DATA:
            header1 = data.pop(0)
            header2 = data.pop(0)
            pkt = NCIDataPacket([header0, header1, header2])

        elif pkt.MT in (pkt.PKT_CTRL_CMD, pkt.PKT_CTRL_RES, pkt.PKT_CTRL_NOT):
            header1 = data.pop(0)
            header2 = data.pop(0)
            pkt = NCIControlPacket([header0, header1, header2])

        payload = []
        for i in range(0, pkt.lenPayload):
            payload.append(data.pop(0))
        pkt.payload = payload

        print(pkt)
