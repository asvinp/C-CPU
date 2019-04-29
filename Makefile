##*****************************************************************************
## CMPE 220 (System Software): Group Assignment
##
## Makefile for CPU Design Assignment (Team 10)
##*****************************************************************************

CC=gcc
CCFLAGS=-g

TARGETS=cpu

all: $(TARGETS)

cpu: cpu_main.o
	$(CC) $(CCFLAGS) -o $@ $^ -lm

cpu_main.o: cpu_main.c cpu_constants.h
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f *.o $(TARGETS)
