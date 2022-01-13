DIR_BUILD	:= build
DIR_INSTALL := /usr/local/bin
BIN_KALK := kalk

CMD_BUILD := cmake
CMD_MKDIR	:= mkdir
CMD_CP := cp
CMD_RM := rm
CMD_CHOWN := chown
CMD_CHMOD := chmod

.PHONY: all
all: build

.PHONY: build
build: prebuild
	$(CMD_BUILD) --build ./$(DIR_BUILD)

.PHONY: prebuild
prebuild:
	$(CMD_MKDIR) -p ./$(DIR_BUILD)
	$(CMD_BUILD) -B ./$(DIR_BUILD)

.PHONY: memcheck
memcheck:
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --error-exitcode=1 ./$(DIR_BUILD)/$(BIN_KALK).out | sed --quiet "/SUMMARY/,$$$$p"

.PHONY: install
install:
	$(CMD_CP) --force ./$(DIR_BUILD)/$(BIN_KALK).out $(DIR_INSTALL)/$(BIN_KALK)
	$(CMD_CHMOD) --reference=$(DIR_INSTALL) $(DIR_INSTALL)/kalk
	$(CMD_CHOWN) --reference=$(DIR_INSTALL) $(DIR_INSTALL)/kalk

.PHONY: uninstall
uninstall:
	$(CMD_RM) --force $(DIR_INSTALL)/$(BIN_KALK)
