# Tarea corta 1: Sincronización de threads

## 1. Compile the program

Navigate to the folder where the source file `main.c` is located and execute:

```bash
$ cd /path/to/solution/folder
$ make
```

## 2. Program execution and available options

### Command

```bash
$ ./main <vehicles-coming-east> <vehicles-coming-west> [<east-spawn-mean>] [<west-spawn-mean>] [<bridge-length>] [<vehicle-speed>]
```

### Arguments

| Argument                      | Description           | Type      | Defaul    | Optional  |
| ------------------------------|-----------------------|-----------|:---------:|:---------:|
| `<vehicles-coming-east>`      | Vehicles coming East  | Integer   |           | No        |
| `<vehicles-coming-west>`      | Vehicles coming West  | Integer   |           | No        |
| `[<east-spawn-mean>]`         | East Spawn Mean       | Double    | 0.25      | Yes       |
| `[<west-spawn-mean>]`         | West Spawn Mean       | Double    | 0.25      | Yes       |
| `[<bridge-length>]`           | Bridge Lenght         | Integer   | 5         | Yes       |
| `[<vehicle-speed>]`           | Vehicle Speed         | Integer   | 1         | Yes       |

## Usage examples

```bash
$ ./main 2 3
$ ./main 2 3 0.3
$ ./main 2 3 0.3 0.5
$ ./main 2 3 0.3 0.5 8
$ ./main 2 3 0.3 0.5 8 2
```

## Help

To get usage help in the terminal execute:

```bash
$ ./main -h
```

---- 

#### Code verification

Code compiled and verified using:

    - Ubuntu 19.10
    - gcc (Ubuntu 9.2.1-9ubuntu2) 9.2.1 20191008
  