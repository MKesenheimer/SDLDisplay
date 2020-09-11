#!/bin/bash

def_host=localhost
def_port=1337

HOST=${2:-$def_host}
PORT=${3:-$def_port}

echo "echo -n \"$1\" | nc -4u -w1 $HOST $PORT"
echo -n "$1" | nc -4u -w1 $HOST $PORT
