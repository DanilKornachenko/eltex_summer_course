FLAGS=-Wall -Werror -Wextra -g

ALL: clean compile run

compile:
	@mkdir -p lib
	@gcc -c -fPIC src/contacts.c -o contacts.o
	@gcc -shared -o ./lib/libcontacts.so contacts.o
	@rm -f *.o
	@gcc $(FLAGS) ./src/main.c -L./lib/ -lcontacts -o main.out

run:
	@LD_LIBRARY_PATH=./lib ./main.out

clean:
	@rm -f main.out
	@rm -f ./lib/*.so
