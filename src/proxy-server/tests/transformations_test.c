#include "../external_transformation.c"

int main(void) {
    char * testPop3 = "+OK 123\r\nhola como ..... estas\r\n.\r\n";
    char * head = malloc(100);
    char * body = malloc(100);
    extract_pop3_info(testPop3, 100, head, body);
    printf("head: %s", head);
    printf("body: %s", body);
    printf("\n SEPARADOR LINEA \n");
    char * normal_text = malloc(100);
    pop3_to_text(body, 100, normal_text);
    printf("parsed text: %s", normal_text);
}