% compute A*x and A^T*x
% where the matrix vector ops are done by a petsc server program 
% and compare results to matlab/octave ops
more off

addpath('client_code/');

% paths and matrix dimensions
pwd_dir = pwd;
petsc_data_dir1 = [pwd_dir,'/petsc_data1/'];
petsc_config_file1 = [pwd_dir,'/petsc_data1/config.txt'];
A_file = [pwd_dir,'/make_test_matrix/diagonalMatrix1.petsc'];
cleanup_cmd = ['./cleanup.pl ', petsc_data_dir1];
m = 1000;
n = 1000;
system(cleanup_cmd);

A = PetscMatrix(A_file,petsc_data_dir1,petsc_config_file1,m,n,pwd_dir);
At = PetscMatrixTranspose(A_file,petsc_data_dir1,petsc_config_file1,m,n,pwd_dir);

A_ref = 2*diag(diag(ones(m,n)));
At_ref = A_ref;


fprintf('do y=A*x\n');
x = rand(n,1);
y = A*x;
whos x y
y_ref = A_ref*x;
perror = 100*norm(y - y_ref)/norm(y_ref);
fprintf('done with mult: norm(x) = %f, norm(y) = %f\n', norm(x), norm(y));
fprintf('perror = %f\n', perror);


fprintf('do yt = At*xt\n');
xt = rand(m,1);
yt = At*xt;
whos xt yt
yt_ref = At_ref*xt;
perror = 100*norm(yt - yt_ref)/norm(yt_ref);
fprintf('done with mult: norm(xt) = %f, norm(yt) = %f\n', norm(xt), norm(yt));
fprintf('perror = %f\n', perror);

