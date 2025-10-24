# SIC-XE Assembler
This SIC-XE assemblercan handle program blocks, but cannot handle control sections due to the required evaluation criterion for even numbered enrollments (23117144). 
## To run:
```bash
$ mkdir -p build
$ cd build
$ cmake ..
$ make
$ cp ../tests/test.sic .
$ ./assembler test.sic
```
Note: If intermediate files are required, then use the `-i` flag like `./assembler -i test.sic`
