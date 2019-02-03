#!/usr/bin/env bash
output=./chip8
olvl=
compiler=clang
src=./src/*.c
libs=-lglew\ -lglfw\ -framework\ OpenGL\ -framework\ CoreVideo\ -framework\ Cocoa\ -framework\ IOKit
flags=-Wall\ -Wextra\ -Wno-switch\ -Wno-unused-function #-DNDEBUG #\ -Werror
std=c99

$compiler $src -std=$std $olvl $libs $flags -o $output -g -fsanitize=address -fno-omit-frame-pointer
