CC = g++
LIBDIR = /usr/local/lib/
BIN = test
SRC = main.cpp \
	  test.hello.pb.cc
LIBS = -lpthread \
	   -lprotobuf
CPPFLAGS += -Wall -Werror -g -O2

all:
	$(CC) $(CPPFLAGS) -L$(LIBDIR) -o $(BIN) $(SRC) $(LIBS)
