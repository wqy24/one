SOURCES=$(wildcard *.test.c)
TARGETS=$(patsubst %.c, %, $(SOURCES))

all: $(TARGETS)


OBJECTS=$(patsubst %.test.c, ../%.o, $(SOURCES))
$(OBJECTS): $(patsubst %.o, %.c, $(OBJECTS))
$(TARGETS): $(OBJECTS)
