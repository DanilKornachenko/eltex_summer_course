FILES := main.c p2p.c

FLAGS := -Wall -Werror -Wextra -g -lrt

all: clean style compile

compile:
	@gcc $(FLAGS) $(FILES) -o main

style:
	@clang-format -style=Google -i *.c
	@clang-format -style=Google -i *.h

clean:
	@rm -f main
