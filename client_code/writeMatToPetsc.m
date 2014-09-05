function writeMatToPetsc(v,fname_petsc,pwd_dir,m,n)
    % add paths
    trans_format_matlab_petsc_prog = [pwd_dir,'/client_code/convertMatlabBinaryVecToPetsc'];
    trans_format_petsc_matlab_prog = [pwd_dir,'/client_code/convertPetscToMatlabBinaryVec'];

    fname_mat = strrep(fname_petsc,'.petsc','.bin');

    % write matlab to binary
    fp = fopen(fname_mat,'w');
    fwrite(fp,v,'double');
    fclose(fp);

    % convert binary to petsc
    cmd_convert = [trans_format_matlab_petsc_prog, ' ', fname_mat, ' ', fname_petsc, ' ', num2str(length(v),'%20f')];
    [status, result] = system(cmd_convert);

    % remove the bin file
    cmd = ['rm -f ', fname_mat];
    [status, result] = system(cmd);
end 
