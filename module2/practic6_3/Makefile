FILES=./src/main.c ./src/calculator.c

FLAGS=-Wall -Werror -Wextra -ldl -g

all: clean compile run

compile: compile_lib
	@gcc $(FLAGS) $(FILES) -o main.out

compile_lib:
	gcc -c -fPIC src/divide.c -o divide.o
	gcc -shared -o ./lib/divide.so divide.o
	gcc -c -fPIC src/mul.c -o mul.o
	gcc -shared -o ./lib/mul.so mul.o
	gcc -c -fPIC src/sub.c -o sub.o
	gcc -shared -o ./lib/sub.so sub.o
	gcc -c -fPIC src/sum.c -o sum.o
	gcc -shared -o ./lib/sum.so sum.o
	rm *.o

run:
	@./main.out

clean:
	@rm -f main.out
	@rm -f ./lib/*.so
