#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// States
#define AUTHORIZATION 0
#define TRANSACTION 1
#define UPDATE 2

// Responses
#define GREETING "+OK ready"

#define USEROK "+OK %s is a valid mailbox"
#define USERERR "-ERR never heard of mailbox %s"

#define PASSOK "+OK maildrop locked and ready"
#define PASSINVALID "-ERR invalid password"
#define PASSMAILALREADYLOCK "-ERR unable to lock maildrop"

#define RETROK "+OK message follows"
#define RETRERR "-ERR no such message"

#define RSETOK "+OK"

#define DELEOK "+OK message deleted"
#define DELEERR "-ERR no such message"

#define NOOPOK "+OK"

#define STATOK "+OK nn mm" //nn mm are two int values

#define LISTOK "+OK scan listing follows"
#define LISTERR "-ERR no such message"

#define PARSERERR "ERROR WHILE PARSING COMANDS"