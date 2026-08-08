FLAGS := -Wall -Werror -Wextra

all: clean compile

compile:
	@gcc $(FLAGS) server.c -o server
	@gcc $(FLAGS) client.c -o client

clean:
	@rm -f server
	@rm -f client
