**
request = opcode resource-id cantParams [params] --> 2bytes 2bytes 2bytes nBytes
	
response = opcode-status --> 1byte 

resource   | id
--------------
concurrent | 0x01
acceses    | 0x02
bytes      | 0x03
mtypes     | 0x04
cmd        | 0x05
**

## GET: 0x02
	- Requests:
		get concurrent
		get accesses
		get bytes
		get mtypes
		get cmd
	- Response:
		get concurrent --> OK [response] --> 2bytes
		get accesses --> OK [response] --> 2bytes
		get bytes --> OK [response] --> 4bytes
		get mtypes --> OK [response] --> 4bytes
		get cmd --> OK [response] --> 4bytes
## SET: 0x03
	- Requests:
		set mtype cantParam [param] // media type
		set cmd cantParam [param] // transformation, default is cat
	- Response:
		OK/ERROR

## RM: 0x04
	- Requests:
		rm mtype cantParam [param]
	- Response:
		OK/ERROR 

## LOGIN: 0x00
	- Request:
		login cantParam [param]
	- Response:
		OK/ERROR

## QUIT: 0x01
	- Request:
		quit
	- Response:
		OK
