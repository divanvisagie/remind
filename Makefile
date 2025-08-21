main: 
	mkdir -p ./bin
	gcc src/main.c -o bin/remind

run: main
	bin/remind

install: main
	mkdir -p ~/.local/bin
	cp bin/remind ~/.local/bin/
	mkdir -p ~/.local/share/man/man1
	cp remind.1 ~/.local/share/man/man1/
