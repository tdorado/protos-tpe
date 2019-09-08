# AUTHORIZATION:

## **USER name**
###    - Argument *name*: Is a required string identifying a mailbox.
###    - Restrictions: May only be given in the AUTHORIZATION state after the POP3 greeting or after an unsuccesful USER or PASS cmd 
###    - Possible Responses: 
        +OK *name* is a valid mailbox
        -ERR never heard of mailbox name

## **PASS string**
###     - Argument *string*: A server/mailbox-specific password (required)
###     - Restrictions: May only be given in the AUTHORIZATION state inmediately after a succesful USER cmd
###     - Posible Responses:
        +OK maildrop locked and ready
        -ERR invalid password
        -ERR unable to lock maildrop

## **APOP name digest**
    En principio no soportamos esto