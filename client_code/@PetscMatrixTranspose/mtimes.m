function y = mtimes(A,x)
addpath('../'); % path to vecWT stuff

% perform petsc mults
y = PetscMatVecOp_one_server_plain(x,A.petsc_config_file1,A.petsc_data_dir1,'matmulttranspose',A.pwd_dir,A.m,A.n); 
