CC = gcc
CFLAGS = -Wall -std=c11 -pedantic -D_DEFAULT_SOURCE -Wno-missing-braces -Wmissing-field-initializers -pthread -Ithirdparty/raylib/include -L. -g
LFLAGS = -lm -lGL -lpthread -lX11 -lraylib
TARGET = leveledit

BASE_SOURCES = base/base_core.c base/base_math.c base/base_strings.c base/base_args.c base/base_log.c base/base_hashtable.c
BASE_OBJS    = $(BASE_SOURCES:.c=.o)

SOURCE_FILES = main.c editor.c entity.c
OBJFILES = $(SOURCE_FILES:.c=.o) $(BASE_OBJS)

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) $(LFLAGS) -o $@

clean:
	rm -rf $(OBJFILES) $(TARGET) *~
