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

readme: remind.1
	./scripts/generate_readme.sh

test-all: test valgrind
	@echo "All tests and checks passed!"

install: main
	mkdir -p ~/.local/bin
	cp bin/remind ~/.local/bin/
	mkdir -p ~/.local/share/man/man1
	cp remind.1 ~/.local/share/man/man1/
