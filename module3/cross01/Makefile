FLAGS := -Wall -Werror -Wextra

all: clean compile

compile:
	gcc $(FLAGS) echo_server.c -o echo_server
	gcc $(FLAGS) echo_client.c -o echo_client

clean:
	rm -f echo_server
	rm -f echo_client
