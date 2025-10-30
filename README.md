# SIC-XE Assembler 
This SIC-XE assembler can handle program blocks, but does not handle control sections.
## To run:
```bash
  mkdir -p build
  cd build
  cmake ..
  make test
  # or instead of the `make test`, type `./assembler -i test.sic` for individual test.
```
Note: If intermediate files are required, then use the `-i` flag like `./assembler -i test.sic` in the build directory.
