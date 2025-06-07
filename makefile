SRCS=$(wildcard *.c)
NAME=Just-in-Check

all:
	gcc $(SRCS) -o $(NAME) -O2
