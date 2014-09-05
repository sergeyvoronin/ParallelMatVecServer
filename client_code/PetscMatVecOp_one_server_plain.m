% multiples [M1] by x or [M1]^T by x using PetscServer for computation
function y=PetscMatVecOp_one_server_plain(x,config_file1,data_dir1,opname,pwd_dir,m,n) 

    % check opname
    if strcmp(opname,'matmult')~=1 && strcmp(opname,'matmulttranspose')~=1
        fprintf('%s is not a valid operation, exiting..\n', opname);
        return;
    end


    if strcmp(opname,'matmult') == 1

        % set up vars
        vec_file_in1_ps1  = [data_dir1, '/x1.petsc'];
        vec_file_out1_ps1 = [data_dir1, '/y1.petsc'];

        x1 = x;


        % write x files to disk for petsc server 1     
        fprintf('writing x1 to %s\n', vec_file_in1_ps1);
        writeMatToPetsc(x1,vec_file_in1_ps1,pwd_dir,m,n);


        % write config file
        fp = fopen(config_file1,'w');
        fprintf(fp,'%s\n', opname);
        fprintf(fp,'%s\n', vec_file_in1_ps1);
        fprintf(fp,'%s\n', vec_file_out1_ps1);
        fclose(fp);

        % read in vec, remove the file and quit
        read_vec = 0;
        while read_vec < 1
            sys_cmd = ['ls -l ', vec_file_out1_ps1];
            [status1, result1] = system(sys_cmd);

            if status1 == 0
                read_vec = 1;
                pause(0.5); % pause to let petsc finish writing
            else
                fprintf('waiting for output\n');
                pause(0.5);
            end 
        end
        fprintf('reading y1 from %s\n', vec_file_out1_ps1);
        y1 = readPetscToMat(vec_file_out1_ps1,pwd_dir,m,n);

        if norm(y1) > 1e30
            pause(3);
            fprintf('re-reading to verify, large norm detected\n');
            fprintf('reading y1 from %s\n', vec_file_out1_ps1);
            y1 = readPetscToMat(vec_file_out1_ps1,pwd_dir,m,n);
            
	    if norm(y1) > 1e30
                fprintf('persistently large norm.. continuing\n');
            end
        end

        % remove file
        cmd_str = ['rm -f ', vec_file_out1_ps1];
        system(cmd_str);


        %%% done calling petsc server 1 %%%

        % finally assemble output y
        y = y1;


    %%%%
    %%%% MATRIX-TRANSPOSE VECTOR MULT 
    %%%%
    elseif strcmp(opname,'matmulttranspose') == 1

        % set up vars
        vec_file_in1_ps1  = [data_dir1, '/x1.petsc'];
        vec_file_out1_ps1 = [data_dir1, '/y1.petsc'];


        % split x according to matrix sizes (do it for both petsc servers)
        x1 = x;


        %%%
        %%% get results from first petsc server
        %%%

        % write x files to disk      
        fprintf('writing x1 to %s\n', vec_file_in1_ps1);
        writeMatToPetsc(x1,vec_file_in1_ps1,pwd_dir,m,n);
        
        % write config file
        fp = fopen(config_file1,'w');
        fprintf(fp,'%s\n', opname);
        fprintf(fp,'%s\n', vec_file_in1_ps1);
        fprintf(fp,'%s\n', vec_file_out1_ps1);
        fclose(fp);

        % read in vec, remove the file and quit
        read_vec = 0;
        while read_vec < 1
            sys_cmd = ['ls -l ', vec_file_out1_ps1];
            [status1, result1] = system(sys_cmd);

            if status1 == 0
                read_vec = 1;
                pause(0.5); % pause to let petsc finish writing
            else
                fprintf('waiting for output\n');
                pause(0.5);
            end 
        end
        fprintf('reading y1 from %s\n', vec_file_out1_ps1);
        y1 = readPetscToMat(vec_file_out1_ps1,pwd_dir,m,n);

        if norm(y1) > 1e30 
            pause(3);
            fprintf('re-reading to verify, large norm detected\n');
            fprintf('reading y1 from %s\n', vec_file_out1_ps1);
            y1 = readPetscToMat(vec_file_out1_ps1,pwd_dir,m,n);

            if norm(y1) > 1e30
                fprintf('persistently large norm.. continuing\n');
            end
        end

        % remove file
        cmd_str = ['rm -f ', vec_file_out1_ps1];
        system(cmd_str);
    
        y = y1;
    end
end
