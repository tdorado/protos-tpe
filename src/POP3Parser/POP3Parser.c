#include "POP3Parser.h"

size_t strlen(const char *s);
int tolower(int c);
int parseUSERCmd(char *input);
int parserPASSCmd(char *input);
void *malloc(size_t size);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
void free(void *ptr);
char *strstr(const char *haystack, const char *needle);

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
    int isPassOk = 0; // si pass ok, entonces pase el authorization state
    
    // parsingCommand and responding
    printf("%s\n", GREETING);
    while(1 && !isPassOk) {
        switch (c = tolower(input[0])) {
            case 'u': // as there is only one command startin with u char, then, it has to be USER cmd
                isUserOk = parseUSERCmd(input);
                break;
            case 'p': // as there is only one command startin with u char, then, it has to be USER cmd
                if (isUserOk) isPassOk = parserPASSCmd(input);
                break;
            default:
                printf("EN DEFAULT, LA CAGUÉ %c\n", c);
                break;
        }
    }
}

// capaz pueda abstraer un poco la logica del parse de cada comando pero me da un toque de paja

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