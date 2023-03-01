# Terminal file manager

Simple terminal file manager implemented on C language with ncurses library.

## Setup

To setup after cloning repository write from main directory
```
mkdir build
cd build
cmake ..
make
```

## Launch

To run manager use
```
./manager <path>
```
from build directory. path - optional field of address to start manager in. If path is not specified, manager starts in current directory.

## Manager options
| Key | Functions |
|-----|-----------|
| enter | Go into directory |
| d | Delete file |
| h | Hide/show hidden files |
| q | Quit |
