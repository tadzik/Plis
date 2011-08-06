CC=clang

plis: plis.c
	$(CC) -ggdb plis.c -o plis -lreadline

clean:
	rm -f plis
