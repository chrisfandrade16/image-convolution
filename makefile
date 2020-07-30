FLAGS=-Wall -Werror -ansi -g

filter: filter.c
	gcc -o filter filter.c $(FLAGS)

clean:
	rm -f filter

