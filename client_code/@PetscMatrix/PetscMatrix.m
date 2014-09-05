function  res = PetscMatrix(A_file,petsc_data_dir1,petsc_config_file1,m,n,pwd_dir)

res.A_file = A_file;
res.petsc_data_dir1 = petsc_data_dir1;
res.petsc_config_file1 = petsc_config_file1;
res.m = m;
res.n = n;
res.pwd_dir = pwd_dir;

% Register this variable as a PetscMatrix class
res = class(res,'PetscMatrix');
