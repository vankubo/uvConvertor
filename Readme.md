# uvConvertor

## Thanks to：

* tinyxml2

* nlohmann/json

* CLI11

## Introduction:

It generate compile_commands.json from .uvprojx file. So the keil MDK project could be indexed by clangd with vscode.

## Usage:

uvConvertor-CLI.exe -f  <path to .uvprojx file> -o <output path> [-e] <external options>

## Build：

It write under C++17,compiled with clang++. You can modify the project in Makefile.

* Bild with cmd line `make`


