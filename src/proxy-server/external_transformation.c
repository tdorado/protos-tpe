#include <stdio.h>
#include <stdlib.h>

#include "include/external_transformation.h"

// HACER FUNCION QUE TOME TEXTO QUE LLEGA DE POP3, HAGA EL PARSEO DE LOS . Y LO MANDE POR UN PIPE 
// A EL STDIN DE UN PROCESO QUE EJECUTE EL CMD QUE ESTA EN PROXY_PARAMS->CMD, Y SE VA A TENER OTRO
// PIPE QUE SE CONECTE CON EL STDOUT DEL PROCESO QUE RECIBE EL TEXTO Y LO VUELVE A PARSEAR CON LOS . EN MODO POP3

char * transform(char * transform_command , char * email) {
    return transform_command;
}