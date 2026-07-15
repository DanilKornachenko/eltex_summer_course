FILES=main.c contacts.c

FLAGS=-Wall -Werror -Wextra

ALL: clean compile run

compile:
	@gcc $(FLAGS) $(FILES) -o main.out

run:
	@./main.out

clean:
	@rm -f main.out
