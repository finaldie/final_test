TARGET = client
BIN_PATH = bin

FLIB_PATH = ../flibs/final_libraries
INCLUDE_PATH = include
LIBS_PATH = libs
INSTALL_PATH = $(FLIB_PATH)/$(BIN_PATH)

CC = gcc
INC = -I$(FLIB_PATH)/$(INCLUDE_PATH)
LL = -L$(FLIB_PATH)/$(LIBS_PATH)
LIBS = -lfhash \
	   -lfmempool \
	   -lpthread \
	   -lflock \
	   -lflog \
	   -lftimer \
	   -lrt \
	   -lflist \
	   -lfev \
	   -lfnet \
	   -lftu \
	   -lfmbuf

SRCS = main.c

COPY_FILES = 
	   
DEPS = $(patsubst %.c,%.d,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(SRCS))

CFLAGS = -Wall -Werror -g

all:$(TARGET)
$(TARGET):$(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(INC) $(LL) -o $(TARGET) $(OBJS) $(LIBS)

%.d:%.c
	$(CC) $(INC) -MM $< > $@
%.o:%.c 
	$(CC) $(CFLAGS) $(INC) -c $< 

.PHONY:clean
clean:
	rm -f $(DEPS) $(OBJS) $(TARGET)

help:
	@echo "The information represents in the program:"
	@echo "Final executable name: $(TARGET)"
	@echo "Source files: $(SRCS)"
	@echo "Object files: $(OBJS)"
depend:$(DEPS)
	@echo "Dependencies are now up-to-date"
-include $(DEPS)
