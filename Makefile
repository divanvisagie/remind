CC = cc

main:
	mkdir -p ./bin
	$(CC) src/main.c -o bin/remind

test-c:
	mkdir -p ./bin
	$(CC) tests/test_remind.c -o bin/test_remind
	./bin/test_remind

debug:
	mkdir -p ./bin
	$(CC) -g -O0 src/main.c -o bin/remind-debug

installer:
	mkdir -p ./bin
	$(CC) src/install.c -o bin/install

run: main
	bin/remind

valgrind: debug
	./scripts/valgrind_tests.sh

test: test-c



docs: remind.1
	./scripts/generate_docs.sh

test-all: test valgrind
	@echo "All tests and checks passed!"

install: main installer
	./bin/install

uninstall:
	@echo "Removing remind from common installation locations..."
	@rm -f ~/.local/bin/remind
	@rm -f ~/.local/share/man/man1/remind.1
	@if [ -w /usr/local/bin ]; then rm -f /usr/local/bin/remind; fi
	@if [ -w /usr/local/share/man/man1 ]; then rm -f /usr/local/share/man/man1/remind.1; fi
	@echo "Uninstall complete"

clean:
	rm -rf ./bin

.PHONY: main debug run valgrind test test-c docs test-all installer install uninstall clean
