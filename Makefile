all: libtags.c tags.c
	gcc -Wall -g -o tags $^ -lsqlite3 -lcrypto -std=c99	

.PHONY: clean

clean:
	rm tags
