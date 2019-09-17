#include "../external_transformation.c"

// this are useless tests, only are used for developing

int main(void) {
    char testPop3[] = "+OK 123\r\nhola como . estas\r\n.\r\n";
    char * head = malloc(100);
    char * body = malloc(100);
    extract_pop3_info(testPop3, 100, head, body);
    printf("head: %s", head);
    printf("body: %s", body);
    printf("\n SEPARADOR LINEA \n");
    char * normal_text = malloc(100);
    pop3_to_text(body, 100, normal_text);
    printf("parsed text: %s", normal_text);
    printf("\n SEPARADOR LINEA \n");
    //char * pop3_text3 = malloc(100);
    //text_to_pop3(normal_text, 100, pop3_text3);
    //printf(" pop3 text: %s", pop3_text3);
    //printf("\n SEPARADOR LINEA \n");
    //printf("\n SEPARADOR LINEA \n");
    //char * transfo = external_transformation("cat", testPop3, 100);
    //printf(" transfo: %s", transfo);
}
