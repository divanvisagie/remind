main:
	mkdir -p ./bin
	gcc src/main.c -o bin/remind

test-c:
	mkdir -p ./bin
	gcc tests/test_remind.c -o bin/test_remind
	./bin/test_remind

debug:
	mkdir -p ./bin
	gcc -g -O0 src/main.c -o bin/remind-debug

run: main
	bin/remind

valgrind: debug
	./scripts/valgrind_tests.sh

test: test-c



readme: remind.1
	./scripts/generate_readme.sh

test-all: test valgrind
	@echo "All tests and checks passed!"

install: main
	./scripts/install.sh

uninstall:
	@echo "Removing remind from common installation locations..."
	@rm -f ~/.local/bin/remind
	@rm -f ~/.local/share/man/man1/remind.1
	@if [ -w /usr/local/bin ]; then rm -f /usr/local/bin/remind; fi
	@if [ -w /usr/local/share/man/man1 ]; then rm -f /usr/local/share/man/man1/remind.1; fi
	@echo "Uninstall complete"

clean:
	rm -rf ./bin

.PHONY: main debug run valgrind test test-c readme test-all install uninstall clean
