# Write-Read-matrix

Description
===

Program creates 2 threads:
writer, who writes 5x5 matrix and incrementing all values by 1.
reader, who reads 5x5 matrix and shows result in the terminal window.
program will ends when user press ctrl+c buttons.

Program using PDI with decl_hdf5 plugin.


Compile
===
Use 'make' command to compile program using Makefile.

Run
===
Use './result' command to run program. 

Description of file
===
Program files are:

main_program.cpp - main program code

matrix_event.yml - event file used by PDI

Makefile - used to compile program

Output files:

data.h5 - matrix data in hdf5 format

result - compiled program output

