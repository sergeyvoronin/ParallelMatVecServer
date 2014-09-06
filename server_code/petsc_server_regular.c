static char help[] = "Reads in a matrix from disk and starts a server waiting for commands and executing them. This version compiles against the regular version of PETSc. Written by Sergey Voronin.";

#include "petscmat.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void choppy( char *s )
{
    s[strcspn ( s, "\n" )] = '\0';
}


#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char **args)
{
    Mat A;
    Vec x,y;
    PetscViewer    pv;               
    PetscErrorCode ierr ;
    PetscInt       m,n,mloc,nloc,server_up,file_scanned,command_executed,vec_size_int; 
    PetscMPIInt    rank, size;
    PetscReal      norm_val;
    char mat_file[PETSC_MAX_PATH_LEN];
    char filename1[PETSC_MAX_PATH_LEN];
    char filename2[PETSC_MAX_PATH_LEN];
    char data_dir[PETSC_MAX_PATH_LEN];
    char config_file[PETSC_MAX_PATH_LEN];
    char command_str[PETSC_MAX_PATH_LEN];
    char *current_working_directory;
    char *uname_info;
    char *date_info;
    char *vec_size_command_str;
    char *vec_size_str;
    FILE *fp;
    time_t start,end;
    double elapsed_time;


    /* initialize petsc */
    PetscInitialize(&argc,&args,(char *)0,help);
    MPI_Comm_size(PETSC_COMM_WORLD,&size);
    MPI_Comm_rank(PETSC_COMM_WORLD,&rank);

    PetscPrintf(PETSC_COMM_SELF,"Number of processors = %d, rank = %d\n",size,rank);	

    /* read in file names */
    ierr = PetscOptionsGetString(PETSC_NULL,"-mat_file",mat_file,PETSC_MAX_PATH_LEN-1,PETSC_NULL);CHKERRQ(ierr);
    ierr = PetscOptionsGetString(PETSC_NULL,"-data_dir",data_dir,PETSC_MAX_PATH_LEN-1,PETSC_NULL);CHKERRQ(ierr);

    /* set up config filename which resides in data directory */
    strcpy(config_file,data_dir); 
    strcat(config_file,"/config.txt"); 

    /* read in matrix from disk */
    PetscPrintf(PETSC_COMM_WORLD,"Loading matrix A from disk: %s\n", mat_file);
    ierr = PetscViewerCreate(PETSC_COMM_WORLD,&pv); CHKERRQ(ierr);
    ierr = PetscViewerSetType(pv, PETSCVIEWERBINARY); CHKERRQ(ierr);
    ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,mat_file,FILE_MODE_READ,&pv);CHKERRQ(ierr);
    ierr = MatCreate(PETSC_COMM_WORLD,&A); CHKERRQ(ierr);
    ierr = MatSetFromOptions(A); CHKERRQ(ierr);
    ierr = MatLoad(A,pv);CHKERRQ(ierr);
    ierr = PetscViewerDestroy(&pv);CHKERRQ(ierr);
    PetscPrintf(PETSC_COMM_WORLD,"done reading.\n");	


    /* get mat sizes */
    ierr  = MatGetSize(A,&m,&n);CHKERRQ(ierr);
    ierr  = MatGetLocalSize(A,&mloc,&nloc);CHKERRQ(ierr);

    PetscPrintf(PETSC_COMM_WORLD,"Read in matrix from file %s.\n", mat_file);	
    PetscPrintf(PETSC_COMM_WORLD,"Global sizes are %d by %d\n", m, n);	
    PetscPrintf(PETSC_COMM_SELF,"Local sizes are %d by %d\n", mloc, nloc);	

    /* get matrix norms */
    MatNorm(A,NORM_INFINITY,&norm_val);
    PetscPrintf(PETSC_COMM_WORLD,"Max norm of A is %f\n",norm_val);
    MatNorm(A,NORM_FROBENIUS,&norm_val);
    PetscPrintf(PETSC_COMM_WORLD,"norm(A,FROBENIUS) = %f\n", norm_val);
    MatNorm(A,NORM_1,&norm_val);
    PetscPrintf(PETSC_COMM_WORLD,"norm(A,1) = %f\n", norm_val);


    /* cleanup, get current working directory and uname -a */
    if(rank==0){
        current_working_directory = (char*)malloc(sizeof(char)*500);
        fp = popen("pwd", "r");
        if(fp == NULL){
            PetscPrintf(PETSC_COMM_SELF,"error executing pwd command\n");
        }
        fgets(current_working_directory,500,fp);
        pclose(fp);

        uname_info = (char*)malloc(sizeof(char)*500);
        fp = popen("uname -a", "r");
        if(fp == NULL){
            PetscPrintf(PETSC_COMM_SELF,"error executing uname command\n");
        }
        fgets(uname_info,500,fp);
        pclose(fp);

        date_info = (char*)malloc(sizeof(char)*100);
        fp = popen("date", "r");
        if(fp == NULL){
            PetscPrintf(PETSC_COMM_SELF,"error executing date command\n");
        }
        fgets(date_info,100,fp);
        pclose(fp);

        choppy(current_working_directory);
        choppy(uname_info);
        choppy(date_info);
    }

    /* loop server ---> */
    server_up = 1;
    file_scanned = 0;
    command_executed = 0;

    // get start time
    time (&start);

    while(server_up == 1){

        // scan file
        if(rank == 0){
            fp = fopen(config_file,"r");
            fscanf(fp,"%s\n",command_str);
            fscanf(fp,"%s\n",filename1);
            fscanf(fp,"%s\n",filename2);
            fclose(fp); 

            PetscPrintf(PETSC_COMM_SELF,"read config file:\n");
            PetscPrintf(PETSC_COMM_SELF,"command_str: %s\n", command_str);
            PetscPrintf(PETSC_COMM_SELF,"filename1: %s\n", filename1);
            PetscPrintf(PETSC_COMM_SELF,"filename2: %s\n", filename2);

            if(strcmp(command_str,"turnoff") == 0){
                PetscPrintf(PETSC_COMM_SELF,"server got turn off signal!\n");
                server_up = 0;
            }
            file_scanned = 1;
        }

        // broadcast needed variables from root processor
        PetscPrintf(PETSC_COMM_WORLD,"broadcasting..\n");
        //file_scanned = 1;
        MPI_Bcast(&file_scanned,1,MPI_LONG_LONG_INT,0,MPI_COMM_WORLD); 
        MPI_Bcast(command_str,PETSC_MAX_PATH_LEN,MPI_CHAR,0,MPI_COMM_WORLD); 
        MPI_Bcast(filename1,PETSC_MAX_PATH_LEN,MPI_CHAR,0,MPI_COMM_WORLD); 
        MPI_Bcast(filename2,PETSC_MAX_PATH_LEN,MPI_CHAR,0,MPI_COMM_WORLD); 
        PetscPrintf(PETSC_COMM_WORLD,"finished broadcasting.\n");

        // synchronize all ranks
        MPI_Barrier (PETSC_COMM_WORLD); 


        // run command specified in file
        if(server_up == 1 && file_scanned == 1){
            // execute current command --->
            if( strcmp(command_str,"pause")  == 0 ){
                PetscPrintf(PETSC_COMM_WORLD,"command: pause --- sleeping ---\n");
                ierr = PetscSleep(0.7); CHKERRQ(ierr);
            }
            // getinfo
            else if( strcmp(command_str,"getinfo") == 0 ){
                PetscPrintf(PETSC_COMM_WORLD,"command: getinfo\n");
                PetscPrintf(PETSC_COMM_WORLD,"writing info to %s\n", filename1);
                time (&end);
                elapsed_time = difftime(end,start);

                PetscPrintf(PETSC_COMM_WORLD,"writing server info to disk\n");
                if(rank == 0){
                    fp = fopen(filename1,"w");
                    fprintf(fp,"%s\n", current_working_directory);
                    fprintf(fp,"%s\n", uname_info);
                    fprintf(fp,"%s\n", date_info);
                    fprintf(fp,"%s\n", mat_file);
                    fprintf(fp,"%d\n", m);
                    fprintf(fp,"%d\n", n);
                    fprintf(fp,"%s\n", config_file);
                    fprintf(fp,"%f\n", elapsed_time);
                    fclose(fp); 
                    command_executed = 1;
                    PetscPrintf(PETSC_COMM_SELF,"rank 0 overwrote file %s --->\n", filename1);
                }
            }

            // matmult
            else if( strcmp(command_str, "matmult") == 0 ){
                
                ierr = PetscSleep(0.1); CHKERRQ(ierr);

                /* check size of input vector! */
                if( rank == 0 ){
                    vec_size_command_str = (char*)malloc(sizeof(char)*200);
                    vec_size_str = (char*)malloc(sizeof(char)*100);
                    strcpy(vec_size_command_str, "server_code/get_petsc_vector_length ");
                    strcat(vec_size_command_str, filename1); 
                    fp = popen(vec_size_command_str, "r");
                    if(fp == NULL){
                        PetscPrintf(PETSC_COMM_SELF,"error executing vecgetsize command\n");
                    }
                    fgets(vec_size_str,100,fp);
                    pclose(fp);
                    PetscPrintf(PETSC_COMM_SELF,"vec_size_str = %s\n", vec_size_str);
                    vec_size_int = atoi(vec_size_str);
                    PetscPrintf(PETSC_COMM_SELF,"vec_size_int = %d\n", vec_size_int);
                    free(vec_size_command_str);
                    free(vec_size_str);
                } 
                MPI_Bcast(&vec_size_int,1,MPI_LONG_LONG_INT,0,MPI_COMM_WORLD); 
                MPI_Barrier (PETSC_COMM_WORLD); 

                if( vec_size_int != n ){
                    PetscPrintf(PETSC_COMM_WORLD,"invalid input length detected - sleeping\n");
                    PetscPrintf(PETSC_COMM_WORLD,"vec_size_int = %d\n", vec_size_int);
                    PetscPrintf(PETSC_COMM_WORLD,"n = %d\n", n);
                    ierr = PetscSleep(5.0); CHKERRQ(ierr);
                    // assume fixed after sleeping, can also loop indefinitely here
                }

                /* read in vector from disk */
                PetscPrintf(PETSC_COMM_WORLD,"command: matmult\n");
                PetscPrintf(PETSC_COMM_WORLD,"filename1 = %s; filename2 = %s\n", filename1, filename2);

                PetscPrintf(PETSC_COMM_WORLD,"create petsc viewer\n");
                ierr = PetscViewerCreate(PETSC_COMM_WORLD,&pv); CHKERRQ(ierr);
                ierr = PetscViewerSetType(pv, PETSCVIEWERBINARY); CHKERRQ(ierr);

                /* set up vector x */
                VecCreate(PETSC_COMM_WORLD,&x);
                VecSetSizes(x,PETSC_DECIDE,n);
                VecSetFromOptions(x); 

                PetscPrintf(PETSC_COMM_WORLD,"Loading vector x from disk: %s\n", filename1);    
                ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,filename1,FILE_MODE_READ,&pv);CHKERRQ(ierr);
                ierr = VecLoad(x,pv);CHKERRQ(ierr);
                ierr = PetscViewerDestroy(&pv);CHKERRQ(ierr);
 
                /* set up vector y */
                PetscPrintf(PETSC_COMM_WORLD,"Setting up vector y: %s\n", filename2);    
                VecCreate(PETSC_COMM_WORLD,&y);
                VecSetSizes(y,PETSC_DECIDE,m);
                VecSetFromOptions(y); 


                /* perform mat-mult */
                PetscPrintf(PETSC_COMM_WORLD,"doing matmult\n");    
                ierr = MatMult(A,x,y); CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"done with matmult\n");    

                /* check norm of y to make sure things are OK */
                ierr = VecNorm(y,NORM_2,&norm_val); CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"norm(y) = %f\n", norm_val);

                /* write y to disk */
                PetscPrintf(PETSC_COMM_WORLD,"writing vector %s.\n", filename2);	
                ierr = PetscViewerCreate(PETSC_COMM_WORLD,&pv); CHKERRQ(ierr);
                ierr = PetscViewerSetType(pv, PETSCVIEWERBINARY); CHKERRQ(ierr);
                ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,filename2,FILE_MODE_WRITE,&pv);CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"Opened viewer.\n");	
                ierr = VecView(y,pv); CHKERRQ(ierr);
                ierr = PetscViewerDestroy(&pv);CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"done writing.\n");	
              
                /* destroy x,y */
                ierr = VecDestroy(&x); CHKERRQ(ierr); 
                ierr = VecDestroy(&y); CHKERRQ(ierr); 

                /* record command execution */
                command_executed = 1; 
            }

            // matmulttranpose
            else if( strcmp(command_str, "matmulttranspose") == 0 ){

                ierr = PetscSleep(0.1); CHKERRQ(ierr);

                /* check size of input vector! */
                if( rank == 0 ){
                    vec_size_command_str = (char*)malloc(sizeof(char)*200);
                    vec_size_str = (char*)malloc(sizeof(char)*100);
                    strcpy(vec_size_command_str, "server_code/get_petsc_vector_length ");
                    strcat(vec_size_command_str, filename1); 
                    fp = popen(vec_size_command_str, "r");
                    if(fp == NULL){
                        PetscPrintf(PETSC_COMM_SELF,"error executing vecgetsize command\n");
                    }
                    fgets(vec_size_str,100,fp);
                    pclose(fp);
                    vec_size_int = atoi(vec_size_str);
                    free(vec_size_command_str);
                    free(vec_size_str);
                } 
                MPI_Bcast(&vec_size_int,1,MPI_LONG_LONG_INT,0,MPI_COMM_WORLD); 
                MPI_Barrier(PETSC_COMM_WORLD);

                if( vec_size_int != m ){
                    PetscPrintf(PETSC_COMM_WORLD,"invalid input length detected - sleeping\n");
                    PetscPrintf(PETSC_COMM_WORLD,"vec_size_int = %d\n", vec_size_int);
                    PetscPrintf(PETSC_COMM_WORLD,"m = %d\n", m);
                    ierr = PetscSleep(5.0); CHKERRQ(ierr);
                }


                /* read in vector from disk */
                PetscPrintf(PETSC_COMM_WORLD,"command: matmulttranspose\n");
                PetscPrintf(PETSC_COMM_WORLD,"filename1 = %s; filename2 = %s\n", filename1, filename2);

                PetscPrintf(PETSC_COMM_WORLD,"create petsc viewer\n");
                ierr = PetscViewerCreate(PETSC_COMM_WORLD,&pv); CHKERRQ(ierr);
                ierr = PetscViewerSetType(pv, PETSCVIEWERBINARY); CHKERRQ(ierr);

                /* set up vector x */
                PetscPrintf(PETSC_COMM_WORLD,"creating vector x of length %d elements\n",m);
                VecCreate(PETSC_COMM_WORLD,&x);
                VecSetSizes(x,PETSC_DECIDE,m);
                VecSetFromOptions(x); 

                PetscPrintf(PETSC_COMM_WORLD,"Loading vector x from disk: %s\n", filename1);    
                ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,filename1,FILE_MODE_READ,&pv);CHKERRQ(ierr);
                ierr = VecLoad(x,pv);CHKERRQ(ierr);
                ierr = PetscViewerDestroy(&pv);CHKERRQ(ierr);
 
                /* set up vector y */
                VecCreate(PETSC_COMM_WORLD,&y);
                VecSetSizes(y,PETSC_DECIDE,n);
                VecSetFromOptions(y); 

                /* perform mat-mult */
                ierr = MatMultTranspose(A,x,y); CHKERRQ(ierr);

                /* check norm of y to make sure things are OK */
                ierr = VecNorm(y,NORM_2,&norm_val); CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"norm(y) = %f\n", norm_val);

                /* write y to disk */
                PetscPrintf(PETSC_COMM_WORLD,"writing vector %s.\n", filename2);	
                ierr = PetscViewerCreate(PETSC_COMM_WORLD,&pv); CHKERRQ(ierr);
                ierr = PetscViewerSetType(pv, PETSCVIEWERBINARY); CHKERRQ(ierr);
                ierr = PetscViewerBinaryOpen(PETSC_COMM_WORLD,filename2,FILE_MODE_WRITE,&pv);CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"Opened viewer.\n");	
                ierr = VecView(y,pv); CHKERRQ(ierr);
                ierr = PetscViewerDestroy(&pv);CHKERRQ(ierr);
                PetscPrintf(PETSC_COMM_WORLD,"done writing.\n");	
              
                /* destroy x,y */
                ierr = VecDestroy(&x); CHKERRQ(ierr); 
                ierr = VecDestroy(&y); CHKERRQ(ierr); 

                /* record command execution */
                command_executed = 1; 
            }

            else{
                PetscPrintf(PETSC_COMM_WORLD,"unrecognized command in file\n");
            }

            // overwrite command file with blank command
            if(command_executed == 1 && rank == 0){
                fp = fopen(config_file,"w");
                fprintf(fp,"pause\n");
                fprintf(fp,"nofile\n");
                fprintf(fp,"nofile\n");
                fclose(fp); 
                command_executed = 0;
                PetscPrintf(PETSC_COMM_SELF,"rank 0 overwrote config file --->\n");
            }
            MPI_Bcast(&command_executed,1,MPI_LONG_LONG_INT,0,MPI_COMM_WORLD); 

            // synchronize all ranks
            MPI_Barrier(PETSC_COMM_WORLD); 

            // reset file scanned status to wait for next command 
            file_scanned = 0;
        } 
        else{
            PetscPrintf(PETSC_COMM_WORLD,"unable to scan config file: %s\n", config_file);
        }
    }

    PetscPrintf(PETSC_COMM_WORLD, "server down, exiting..\n");
  
    /* destroy the mat and vec */
    ierr = MatDestroy(&A); CHKERRQ(ierr);
    ierr = VecDestroy(&x); CHKERRQ(ierr);

    /* finalize and exit */
    ierr = PetscFinalize();CHKERRQ(ierr);
    return 0;
}

