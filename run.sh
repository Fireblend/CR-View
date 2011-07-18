#!/usr/bin/env bash

cd .
make clean
sleep 1
make
./cr-view
