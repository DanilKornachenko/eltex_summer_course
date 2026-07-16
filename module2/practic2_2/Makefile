FILES=main.c calculator.c

FLAGS=-Wall -Werror -Wextra

all: clean compile run

compile:
	@gcc $(FLAGS) $(FILES) -o main.out

run:
	@./main.out

clean:
	@rm -f main.out
