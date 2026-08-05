FILES=main.c broker.c
FLAGS=-Wall -Werror -Wextra

all: clean style compile

compile:
	@gcc -g $(FILES) -o main

clean:
	@rm -f main

style:
	@clang-format -style=Google -i *.c
	@clang-format -style=Google -i *.h
