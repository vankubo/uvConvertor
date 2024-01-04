# uvConvertor

## Thanks to：

* tinyxml2

* nlohmann/json

* CLI11

## Introduction:

It generate compile_commands.json from .uvprojx and .dep file. So the keil MDK project could be indexed by clangd with vscode.

## Usage:

1. Compile project

2. Execute Command

```bash
uvConvertor-CLI.exe -f <path to .uvprojx file> -o <output path> [-e] <external options>
```


## Build：

### Makefile

It write under C++17,compiled with clang++. You can modify the project in Makefile.

* Bild with cmd line `make`

### Cmake

* Go into folder `build`

* `cmake .. -G"Unix Makefiles"`

* `make`

### Visual Studio

* Go into folder `build`

* `cmake .. -G"Visual Studio 17 2022"`

* use Visual Studio open `main.sln`
