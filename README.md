# ProxyPOP3Filter
## TPE Protocolos de Comunicación 2019 2 Cuatrimestre

### Código fuente
El código fuente del trabajo se encuenta dentro de la directorio src:
```sh
src/
```

### Documentación
Tanto las consignas del trabajo como manual, informe y presentación se encuentan en el directorio:
```sh
docs/
```

### Instrucciones de compilación
En el directorio principal del proyecto realizar:
```sh
$ make
```
Esto generara los ejecutables:
  - pop3filter
  - pop3ctl
  - stripmime

### Ejecución
Para ejecutar pop3filter:
```sh
$ ./pop3filter [POSIX Style Options] <origin-server-address>
```

Para ejecutar adminclient:
```sh
$ ./pop3ctl -a <management-address> -p <management-port>
```
Siendo ambas opciones no necesarias, si no se indican, se tomarán los valores por defecto que son 127.0.0.1 de managment-address y 9090 de managment-port.

Para ejecutar el stripmime por su cuenta:
Se debe configurar las variables de entorno:

```sh
"FILTER_MEDIAS"
"FILTER_MSG"
"POP3FILTER_VERSION"
"POP3_USERNAME"
"POP3_SERVER"
```

```sh
$ ./stripmime
```

### Opciones
Para ver todas las opciones ejecutar:
```sh
$ ./pop3filter -h
```

## Integrantes
- Battilana, Joaquín
- Dallas, Tomás
- Dorado, Tomás
- Princ, Guido