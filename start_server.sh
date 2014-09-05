#!/bin/bash

source setup_vars.sh
./cleanup.pl petsc_data1/
mpirun -np 12 server_code/petsc_server_regular -mat_file make_test_matrix/diagonalMatrix1.petsc -data_dir petsc_data1/ &
