# Makefile for printmon
# Copyright (C) 1993 Kai Uwe Rommel <rommel@ars.muc.de>

CC = gcc -Zomf -Zmtd -O -s
O = .obj
CFLAGS = -D_System=

TCPINCDIR = d:/tcpip/include
TCPLIBDIR = d:/tcpip/lib

.SUFFIXES: .c $O

.c$O:
	$(CC) $(CFLAGS) -I$(TCPINCDIR) -c $<

all: printmon.exe hpprint.exe

printmon.exe: printmon$O
	$(CC) -o $@ printmon$O

hpprint.exe: hpprint$O
	$(CC) -o $@ hpprint$O -L $(TCPLIBDIR) -lso32dll -ltcp32dll
