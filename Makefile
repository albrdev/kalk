DIR_BUILD	:= build

CMD_MKDIR	:= mkdir -p

.PHONY: all
all: build

.PHONY: build
build: generate
	cmake --build ./$(DIR_BUILD)

.PHONY: generate
generate:
	mkdir -p ./$(DIR_BUILD)
	cmake -B ./$(DIR_BUILD)

.PHONY: unittest
unittest:
	@./build/test/unit_tests

.PHONY: memcheck
memcheck:
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --error-exitcode=1 ./build/test/unit_tests 2>&1 | sed -n "/SUMMARY/,$$$$p"
