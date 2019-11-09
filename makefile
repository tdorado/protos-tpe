PROXY_NAME = pop3filter
PROXY_DIR = src/proxy-server
PROXY_SOURCES = $(PROXY_DIR)/main $(PROXY_DIR)/settings $(PROXY_DIR)/logs $(PROXY_DIR)/external_transformation $(PROXY_DIR)/admin_socket $(PROXY_DIR)/metrics $(PROXY_DIR)/origin_server_socket $(PROXY_DIR)/proxy_clients $(PROXY_DIR)/proxy_socket $(PROXY_DIR)/error_file $(PROXY_DIR)/buffer $(PROXY_DIR)/utils

CC_C = gcc
CFLAGS = -Wall
CFLAGSBIEN = -Wall -Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-prototypes -Wcast-align -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Werror -pedantic-errors -Wmissing-prototypes -pedantic -std=c99 -D_DEFAULT_SOURCE
SCTPFLAGS = -lsctp
PTHREADFLAG = -pthread
DEBUGFLAGS = -g

all: clean proxy-server

clean:
	@rm -f $(PROXY_NAME) $(PROXY_MAIN:=.o)
	@rm -f $(PROXY_SOURCES) $(PROXY_SOURCES:=.o)

proxy-server:
	$(CC_C) $(CFLAGS) $(SCTPFLAGS) $(PTHREADFLAG) $(DEBUGFLAGS) $(PROXY_SOURCES:=.c) -o $(PROXY_NAME) -lrt
	
.PHONY: all