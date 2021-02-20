parser: parser.c
	gcc -std=gnu11 -Werror -Wall -o parser parser.c -g
clean: parser
	rm parser
