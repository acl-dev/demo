#!/bin/sh

valgrind --tool=memcheck --leak-check=yes -v ../bin/vstring_base64
