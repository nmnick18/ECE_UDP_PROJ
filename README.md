# ECE_UDP_PROJ
In this project, we're advancing our knowlege in the field of socket programming. Initially we wanted to down the route of using TCP, but we realized that it was not very easy to use.

We decided to use our old lab 5 to get this working.

The objective of this project was to use UDP to send files, we learned that in order to do this, we must know more about sending a buffer of a packet. We would need to send multiple packets in order for the other side to receive those packets.

## UDP project 

Team members:

Wills Stull(client)
Nikhil Malani(client)
Raj Chaudhari(server)
Segev Apter(server)
Jeffrey Murray(CLI)

<------------------------------------------------------------------>
below is the instruction on how to use our project

### Install

Run the following command

```
make
``` 

### use 

Once this is ran, use the following command on two seperate terminal windows

```
./UDP_FTP.out 4545
./UDP_FTP.out localhost 4545
```


After this is done we using the client to send the following:
```
command: put <file-path>
```
## Task

- [x] Get Server working
- [x] Get client connected to server
- [x] work on CLI component
- [x] Documentation
- [x] Work statement
