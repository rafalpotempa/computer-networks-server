# Computer Networks Server <!-- omit in toc -->

- [Description](#description)
- [Compile](#compile)
- [Usage](#usage)
	- [Run server](#run-server)
	- [Make connection](#make-connection)

## Description

Exercise to create simple UNIX echo server in C.

## Compile

```shs
gcc -o cn-server server/server_deamon.c
```

## Usage

### Run server

```sh
./cn-server
```

### Make connection

```sh
echo "test" | nc localhost 1234
```
