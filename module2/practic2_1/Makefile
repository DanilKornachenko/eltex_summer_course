FILES=main.c contacts.c

TEST_FILES=contacts.c unit_tests.c

FLAGS=-Wall -Werror -Wextra -g

ALL: clean compile run

compile:
	@gcc $(FLAGS) $(FILES) -o main.out

run:
	@./main.out

test: clean
	@gcc $(FLAGS) -lcheck $(TEST_FILES) -o test.out
	@./test.out

clean:
	@rm -f main.out
	@rm -f test.out
