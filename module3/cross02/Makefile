FILES := main.c drivers.c
FLAGS := -Wall -Werror -Wextra

all: clean style compile

compile:
	@gcc $(FLAGS) $(FILES) -o drivers.out

style:
	@clang-format -style=Google -i *{.c,.h}

clean:
	@rm -f drivers.out
