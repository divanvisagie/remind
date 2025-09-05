main:
	mkdir -p ./bin
	gcc src/main.c -o bin/remind

debug:
	mkdir -p ./bin
	gcc -g -O0 src/main.c -o bin/remind-debug

run: main
	bin/remind

valgrind: debug
	./scripts/valgrind_tests.sh

test: main
	./scripts/simple_test.sh

install: main
	mkdir -p ~/.local/bin
	cp bin/remind ~/.local/bin/
	mkdir -p ~/.local/share/man/man1
	cp remind.1 ~/.local/share/man/man1/
