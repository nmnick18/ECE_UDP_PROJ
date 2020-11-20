# ECE 4122
# FTP

CC = gcc

make: client server
	g++ client.cpp -o client.out
	g++ server.cpp -o server.out