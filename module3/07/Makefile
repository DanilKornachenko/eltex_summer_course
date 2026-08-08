FLAGS := -Wall -Werror -Wextra

all: clean style compile

compile:
	@gcc $(FLAGS) server.c -o server
	@gcc $(FLAGS) client.c -o client

style:
	@clang-format -style=Google -i *.c

clean:
	@rm -f server
	@rm -f client
