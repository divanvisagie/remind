main: 
	gcc src/main.c -o bin/remind

run: main
	bin/remind

install: main
	cp bin/remind /usr/local/bin/
	cp remind.1 /usr/local/share/man/man1/
