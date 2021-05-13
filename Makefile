CC=gcc
TARGET=minishell

all:
	$(CC) minishell.c -o minishell -lreadline
clean:
	rm -rf minishell
