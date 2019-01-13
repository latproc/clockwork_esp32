This directory contains some simple clockwork programs that are used
as examples for the generation of C code from clockwork.

to run them, first compile the clockwork interpreter 

  cd latproc/clockwork/iod
  make

then run the interpreter passing this directory and an option to trigger the export:

 ./tools/build <your cw code>

a -f will also flash the ESP with the code
 ./tools/build <your cw code>
