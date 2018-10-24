#!/usr/bin/env python 

"""
An echo server that uses select to handle multiple clients at a time. 
Entering any line of input at the terminal will exit the server. 
"""

import select
import socket
import sys 

host = ''
port_nav = 8080
port = 50000
backlog = 5
size = 1024

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((host,port))
server.listen(backlog)

server_nav = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_nav.bind((host,port_nav))
server_nav.listen(backlog)

input = [server, server_nav, sys.stdin]

file_obj  = open('tabla.html', 'r')
tabla = file_obj.read()

data_temp = ['0','0']
data_light = ['0','0']
data_noise = ['0','0']
#date = ['0','0']

running = 1
while running:
	inputready,outputready,exceptready = select.select(input, [], [] )

	for s in inputready:
		if s == server:
			# handle the server socket
			client, address = server.accept()
			input.append(client)
		elif s == server_nav:
			# handle the server_nav socket
			client, address = server_nav.accept()
			input.append(client)
		elif s == sys.stdin:
			# handle standard input
			junk = sys.stdin.readline()
			running = 0
		else:
			# handle all other sockets (client)
			print("Client")
			data = s.recv(size) 
			
			if (data.find('host', 0, 10) != -1):
				#print data.split(' ')
				junk, node, temperature, light, noise = data.split(' ')
				if temperature != '-':
					data_temp[int(node)-1] = temperature
				if light != '-':
					data_light[int(node)-1] = light
				if noise != '-':
					data_noise[int(node)-1] = noise
				s.send(data)
				s.close()
				input.remove(s)

			else:
				tabla_temp = tabla
				for i in range(2):
					tabla_temp = str(tabla_temp).replace('dato'+str(i+1)+ '1', data_temp[i])
					tabla_temp = str(tabla_temp).replace('dato'+str(i+1)+ '2', data_light[i])				
					tabla_temp = str(tabla_temp).replace('dato'+str(i+1)+ '3', data_noise[i])
				s.send(tabla_temp)			
				s.close()
				input.remove(s)

server.close()
server_nav.close()

	
