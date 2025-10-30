# SIC-XE Assembler 
This SIC-XE assembler can handle program blocks, but does not handle control sections (23117144- Even enrollment number) 
## To run:
```bash
  mkdir -p build
  cd build
  cmake ..
  make test
  # or instead of the `make test`, type `./assembler -i test.sic` for individual test.
```
The example listing file and object file for the required test are added to the `tests/` folder.  
Note: If intermediate files are required, then use the `-i` flag like `./assembler -i test.sic` in the build directory.
