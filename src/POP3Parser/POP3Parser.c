#include "POP3Parser.h"

size_t strlen(const char *s);
int tolower(int c);void *malloc(size_t size);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
void free(void *ptr);
char *strstr(const char *haystack, const char *needle);

int parseUSERCmd(char *input);
int parserPASSCmd(char *input);
void parseQUITCmd();
void parseRETRorRSETCmd(char *input);
void parseRETRCmd(char *input);
void parserRSETCmd(char *input);
void parseDELECmd(char *input);
void parseNOOPCmd(char *input);
void parseSTATCmd(char *input);
void parseLISTCmd(char *input);

// asumo que recibo siempre un char[] de algún lado
int main() {    
    
    char *input = "USER tdallas";
    // char input[] = "USER tdallas asdlkj";
    // char input[] = "PASS hola";
    // char input[] = "PASS hola asdlkj";
    // char input[] = "PASS hola ";
    // char input[] = "USER tdallas ";

    int i = 0;
    char c;

    int isUserOk = 0;
    int isPassOk = 0; // si pass ok, entonces pasé el authorization state
    
    // parsingCommand and responding
    // STAT                    valid in the TRANSACTION state
    //   LIST [msg]
    //   RETR msg
    //   DELE msg
    //   NOOP
    //   RSET

    printf("%s\n", GREETING);
    while(1 /* && !isPassOk aca va algun flag cheto*/ ) {
        switch (c = tolower(input[0])) {
            case 'u': // as there is only one command startin with u char, then, it has to be USER cmd
                isUserOk = parseUSERCmd(input);
                break;
            case 'p': // as there is only one command startin with u char, then, it has to be USER cmd
                if (isUserOk) isPassOk = parserPASSCmd(input);
                break;
            case 'q':
                parseQUITCmd();
                break;
            case 's': //STAT
                if (isPassOk) parseSTATCmd(input);
                break;
            case 'l': //LIST [msg] es optional, refiere a un msg-number
                if (isPassOk) parseLISTCmd(input);
                break;
            case 'r': //RETR msg o RSET
                if (isPassOk) parseRETRorRSETCmd(input);
                break;
            case 'd': // DELE msg
                if (isPassOk) parseDELECmd(input);
                break;
            case 'n': //NOOP
                if (isPassOk) parseNOOPCmd(input);
                break;
            default:
                printf("EN DEFAULT, LA CAGUÉ %c\n", c);
                break;
        }
    }
}

// capaz pueda abstraer un poco la logica del parse de cada comando pero me da un toque de paja

void parseRETRorRSETCmd(char *input){
    if (((char) tolower(input[1])) == 'e') {
        parseRETRCmd(input);
    } else if (((char) tolower(input[1])) == 's') {
        parserRSETCmd(input);
    } else {
        //error
    }
}

void parseQUITCmd() {
    // TODO quitea vieja
}

int parseUSERCmd(char *input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;
    // check whether its actually USER command
    if (!strncmp(input, "USER ", 5)) { 
        strncpy(arguments, input+5, inputLength-4);
        // check wheter is not 1 argument
        if (strstr(arguments, " ") != NULL) error = 1;
        // if its one argument, we should check somewhere else this argument
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    char *str = malloc(strlen(USERERR) + strlen(arguments) +1);
    sprintf(str, error ? USERERR : USEROK, arguments);
    printf("%s\n", str);

    free(arguments);

    return !error; 
}

int parserPASSCmd(char* input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually USER command
    if (!strncmp(input, "PASS ", 5)) {
        strncpy(arguments, input+5, inputLength-4);
        // check wheter is not 1 argument
        if (strstr(arguments, " ") != NULL) error = 1;
        // if its one argument, we should check somewhere else this argument
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }
    
    printf("%s\n", error ? PASSINVALID : PASSOK);
    free(arguments);

    return !error;
}

void parseRETRCmd(char *input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually RETR command
    if (!strncmp(input, "RETR ", 5)) { 
        strncpy(arguments, input+5, inputLength-4);
        // check wheter is not 1 argument
        if (strstr(arguments, " ") != NULL) error = 1;
        // if its one argument, we should check somewhere else this argument
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    char *str = malloc(strlen(RETRERR) + strlen(arguments) +1);
    sprintf(str, error ? RETRERR : RETROK, arguments);
    printf("%s\n", str);

    free(arguments);
}

void parserRSETCmd(char *input) { 
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually RSET command
    if (!strncmp(input, "RSET ", 5) ) { 

    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    //print whatever goes in stat cmd
    char *str = malloc(strlen(RSETOK) + strlen(arguments) +1);
    sprintf(str, RSETOK, arguments);
    printf("%s\n", str);

    free(arguments);
}

void parseDELECmd(char *input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually DELE command
    if (!strncmp(input, "DELE ", 5)) { 

        strncpy(arguments, input+5, inputLength-4 );
        // check wheter is not 1 argument
        if (strstr(arguments, " ") != NULL) error = 1;
        // if its one argument, we should check somewhere else this argument
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    char *str = malloc(strlen(DELEERR) + strlen(arguments) +1);
    sprintf(str, error ? DELEERR : DELEOK, arguments);
    printf("%s\n", str);

    free(arguments);
}

void parseNOOPCmd(char *input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually NOOP command
    if (!strncmp(input, "NOOP ", 5)) { 
        // estamos ok, stat no recibe argumentos
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    //print whatever goes in stat cmd
    char *str = malloc(strlen(NOOPOK) + strlen(arguments) +1);
    sprintf(str, NOOPOK, arguments);
    printf("%s\n", str);

    free(arguments);
}

void parseSTATCmd(char *input) { 
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;

    // check whether its actually STAT command
    if (!strncmp(input, "STAT ", 5)) { 
        // estamos ok, stat no recibe argumentos
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    //print whatever goes in stat cmd
    char *str = malloc(strlen(STATOK) + strlen(arguments) +1);
    sprintf(str, STATOK, arguments);
    printf("%s\n", str);

    free(arguments);
}

void parseLISTCmd(char *input) {
    int inputLength = strlen(input);
    char *arguments = malloc(inputLength);
    int error = 0;
    // check whether its actually LIST command
    if (!strncmp(input, "LIST ", 5)) { 
        if (inputLength == 5) { // only one argument
            
        }

        strncpy(arguments, input+4, inputLength-3);
        // check wheter is not 1 argument
        if (strstr(arguments, " ") != NULL) error = 1;
        // if its one argument, we should check somewhere else this argument
    } else {
        // error
        printf("El comando que le pasaste esta al mostro\n");
    }

    char *str = malloc(strlen(LISTOK) + strlen(arguments) +1);
    sprintf(str, error ? LISTERR : LISTOK, arguments);
    printf("%s\n", str);

    free(arguments);
}