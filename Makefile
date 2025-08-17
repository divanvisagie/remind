main: 
	gcc src/main.c -o bin/remind

run: main
	bin/remind

install: main
	cp bin/remind /usr/local/bin/
