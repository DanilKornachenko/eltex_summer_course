FLAGS=-Wall -Werror -Wextra -g

ALL: clean compile run

compile:
	@gcc -c ./src/contacts.c -o contacts.o
	@ar rcs ./lib/libcontacts.a contacts.o
	@rm -f *.o

	@gcc $(FLAGS) ./src/main.c -L./lib/ -lcontacts -o main.out

run:
	@./main.out

clean:
	@rm -f main.out
	@rm -f ./lib/*.a
