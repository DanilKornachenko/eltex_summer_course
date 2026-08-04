
all: clean compile check clean check2 clean

compile:
	@gcc -Wall -Werror -Wextra -g main.c -o main

clean:
	@rm -f main
	@rm -f pipe
	@rm -f test_files/hello.txt.copy
	@rm -f test_files/how_are_you.log.copy

check:
	@./main test_files/hello.txt test_files/how_are_you.log -p pipe
	diff test_files/hello.txt test_files/hello.txt.copy
	diff test_files/how_are_you.log test_files/how_are_you.log.copy

check2:
	@./main test_files/hello.txt test_files/how_are_you.log
	diff test_files/hello.txt test_files/hello.txt.copy
	diff test_files/how_are_you.log test_files/how_are_you.log.copy
