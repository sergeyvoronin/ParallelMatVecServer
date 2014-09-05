#!/bin/bash

source setup_vars.sh

gcc -o make_test_matrix/write_diagonal_matrix make_test_matrix/write_diagonal_matrix.c 

gcc -o client_code/convertMatlabBinaryVecToPetsc client_code/convertMatlabBinaryVecToPetsc.c
gcc -o client_code/convertPetscToMatlabBinaryVec client_code/convertPetscToMatlabBinaryVec.c

make get_petsc_vector_length
make petsc_server_regular
