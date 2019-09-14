PROXY_NAME = pop3filter
PROXY_SOURCES = src/proxy-server/main src/proxy-server/input_parser src/proxy-server/logs src/proxy-server/external_transformation

CC_C = gcc
CFLAGS = -Wall -g -pthread -std=gnu99

all: clean proxy-server

clean:
	@rm -f $(PROXY_NAME) $(PROXY_MAIN:=.o)
	@rm -f $(PROXY_SOURCES) $(PROXY_SOURCES:=.o)

proxy-server:
	$(CC_C) $(CFLAGS) $(PROXY_SOURCES:=.c) -o $(PROXY_NAME) -lrt
	
.PHONY: all