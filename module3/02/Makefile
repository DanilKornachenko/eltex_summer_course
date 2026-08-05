FILES=main.c broker.c
FLAGS=-Wall -Werror -Wextra

all: clean compile
	@./main -b

compile:
	@gcc -g $(FILES) -o main

clean:
	@rm -f main
