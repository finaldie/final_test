CC = g++
LIBDIR = /usr/local/lib/
BIN = test
SRC = main.cpp \
	  test.hello.pb.cc
LIBS = -lpthread \
	 -lprotobuf

all:
	$(CC) -Wall -g -L$(LIBDIR) -o $(SRC) $(SRC) $(LIBS)
