#INSTALLATION

##Compilation

`$ ./autocrap.sh
 $ ./configure
 $ make all
`

Using make with no arguments should build everything, but to avoid the compilation of auxiliary tools, data generation,
and everything you might want to try:

`$ make programs`

which will compile only binaries.

Note: does not compile with Clang compiler currently, requires g++

## Installation

Try

`# make install`

which will install all binaries, and libraries.
