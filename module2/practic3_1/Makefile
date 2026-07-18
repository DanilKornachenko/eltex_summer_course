FILES=main.c stat.c

FLAGS=-Wall -Werror -Wextra

all: run

compile: clean
	@gcc $(FLAGS) $(FILES) -o stat.out

run: clean compile
	@./stat.out

clean:
	@rm -f stat.out
