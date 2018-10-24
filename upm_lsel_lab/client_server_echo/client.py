#!/usr/bin/env python

"""
A client that sends one or more values of sensed data to a host,
identifying itself by a node_number.
"""

import socket
import argparse

__author__ = 'Daniel Gala Montes'

# Assign description for documentation
parser = argparse.ArgumentParser(description='Send node data to server.')

# Add arguments
parser.add_argument('host', type=str, help='server_host')
parser.add_argument('node', type=str, help='node_number')
parser.add_argument('-t', '--temperature', type=str, help='temperature_value')
parser.add_argument('-l', '--light', type=str, help='light_value')
parser.add_argument('-n', '--noise', type=str, help='noise_value')

# Array for all the arguments passed in
args = parser.parse_args()
if not (args.temperature or args.light or args.noise):
    parser.error('No data specified. Please provide at least one variable')
else:
    #Assign args to variables
    host = args.host
    node = args.node
    temperature = args.temperature
    light = args.light
    noise = args.noise

    # Join the arguments into a single message
    seq = (host + ' ' + node + ' ')
    if (temperature):
    	seq = seq + (temperature + ' ')
    else:
	seq = seq + ('- ')
    if (light):
    	seq = seq + (light + ' ')
    else:
	seq = seq + ('- ')
    if (noise):
    	seq = seq + (noise)
    else:
	seq = seq + ('-')
    #message = " "
    #message = message.join(seq)
    #print seq
    port = 50000
    size = 1024
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host,port))
    s.send(seq)
    data = s.recv(size)
    s.close()
    print data
