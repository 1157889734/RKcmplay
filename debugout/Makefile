#include ../build.cfg

TAR_FILE=libdebugout.a
HEAD_FILE=debug.h
RESUME_DIR_FILES += -path ./demo -prune -o

SRC_FILES = $(shell find ./  $(RESUME_DIR_FILES) -name "*.c" -print)
OBJ_FILES=$(patsubst %.c,%.o,$(SRC_FILES))

all:$(TAR_FILE)
$(TAR_FILE):$(OBJ_FILES)
	$(AR) -rc $(TAR_FILE) $(OBJ_FILES)
	cp $(TAR_FILE) ../lib/
	cp $(HEAD_FILE) ../include

install:
	cp $(TAR_FILE) ../lib/
	cp $(HEAD_FILE) ../include
clean:
	rm -rf $(OBJ_FILES)
	rm -rf $(TAR_FILE)
