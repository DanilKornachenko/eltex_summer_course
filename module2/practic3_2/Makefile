FILES=main.c ping.c
FLAGS=-Wall -Werror -Wextra

all: run

run: compile
	@./ping.out

compile: clean
	@gcc $(FLAGS) $(FILES) -o ping.out

clean:
	@rm -f ping.out
