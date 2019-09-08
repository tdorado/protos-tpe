#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// States
#define AUTHORIZATION 0

// Responses
#define GREETING "+OK ready"

#define USEROK "+OK %s is a valid mailbox"
#define USERERR "-ERR never heard of mailbox %s"

#define PASSOK "+OK maildrop locked and ready"
#define PASSINVALID "-ERR invalid password"
#define PASSMAILALREADYLOCK "-ERR unable to lock maildrop"