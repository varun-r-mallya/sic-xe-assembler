# SIC-XE Assembler 
This SIC-XE assembler can handle program blocks, but does not handle control sections. 
## To run:
```bash
  mkdir -p build
  cd build
  cmake ..
  make test
  
```
Note: If intermediate files are required, then use the `-i` flag like `./assembler -i test.sic`
