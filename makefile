PROXY_NAME = pop3filter
PROXY_DIR = src/proxy-server
PROXY_SOURCES = $(PROXY_DIR)/main $(PROXY_DIR)/settings $(PROXY_DIR)/logs $(PROXY_DIR)/external_transformation $(PROXY_DIR)/admin_socket $(PROXY_DIR)/metrics $(PROXY_DIR)/origin_server_socket $(PROXY_DIR)/proxy_clients $(PROXY_DIR)/proxy_socket $(PROXY_DIR)/error_file $(PROXY_DIR)/buffer $(PROXY_DIR)/utils $(PROXY_DIR)/pop_parser $(PROXY_DIR)/admin_thread $(PROXY_DIR)/admin_parser

ADMIN_CLIENT_NAME = pop3ctl
ADMIN_CLIENT_DIR = src/admin-client
ADMIN_CLIENT_SOURCES = $(ADMIN_CLIENT_DIR)/main $(ADMIN_CLIENT_DIR)/admin_client_parser

STRIPMIME_NAME = stripmime
STRIPMIME_DIR = src/stripmime
STRIPMIME_SOURCES = $(STRIPMIME_DIR)/stripmime $(STRIPMIME_DIR)/utils/stripmime_queue

CC_C = gcc
CFLAGS = -Wall -Wextra -Wfloat-equal -Wpointer-arith -Wstrict-prototypes -Wcast-align -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Werror -pedantic-errors -Wmissing-prototypes -pedantic -std=c99 -D_DEFAULT_SOURCE
SCTPFLAGS = -lsctp
PTHREADFLAG = -pthread
DEBUGFLAGS = -g

all: clean proxy-server admin-client stripmime

clean:
	@rm -f $(PROXY_NAME)
	@rm -f $(PROXY_SOURCES) $(PROXY_SOURCES:=.o)
	@rm -f $(ADMIN_CLIENT_NAME)
	@rm -f $(ADMIN_CLIENT_SOURCES) $(ADMIN_CLIENT_SOURCES:=.o)
	@rm -f $(STRIPMIME_NAME)
	@rm -f $(STRIPMIME_SOURCES) $(STRIPMIME_SOURCES:=.o)

proxy-server:
	$(CC_C) $(CFLAGS) $(DEBUGFLAGS) $(PROXY_SOURCES:=.c) -o $(PROXY_NAME) $(SCTPFLAGS) $(PTHREADFLAG) -lrt

admin-client:
	$(CC_C) $(CFLAGS) $(DEBUGFLAGS) $(ADMIN_CLIENT_SOURCES:=.c) -o $(ADMIN_CLIENT_NAME) $(SCTPFLAGS) $(PTHREADFLAG) -lrt

stripmime:
	$(CC_C) $(CFLAGS) $(DEBUGFLAGS) $(STRIPMIME_SOURCES:=.c) -o $(STRIPMIME_NAME) $(PTHREADFLAG) -lrt
	
.PHONY: all