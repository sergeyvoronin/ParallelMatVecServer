% reads petsc file from fname_petsc
function v = readPetscToMat(fname_petsc,pwd_dir,m,n)
    % add paths
    trans_format_matlab_petsc_prog = [pwd_dir,'/client_code/convertMatlabBinaryVecToPetsc'];
    trans_format_petsc_matlab_prog = [pwd_dir,'/client_code/convertPetscToMatlabBinaryVec'];


    fname_mat = strrep(fname_petsc,'.petsc','.bin');

    cmd_convert = [trans_format_petsc_matlab_prog, ' ', fname_petsc, ' ', fname_mat];
    [status, result] = system(cmd_convert);

    fp = fopen(fname_mat,'r');
    v = fread(fp,n,'double'); % NOTE: for Octave, size needed to read back large file!
    fclose(fp);

    % remove the bin file
    cmd = ['rm -f ', fname_mat];
    [status, result] = system(cmd);
end 
