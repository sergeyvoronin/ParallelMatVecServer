/* sample program for converting binary vector written by matlab to PETSc format without using mex files
gcc -o convertMatlabBinaryVecToPetsc convertMatlabBinaryVecToPetsc.c 

use like this:
vec_file_mat  = [data_dir, '/x.bin'];
vec_file_petsc  = [data_dir, '/x.petsc'];

fprintf('writing x to %s\n', vec_file_petsc);
fp = fopen(vec_file_mat,'w');
fwrite(fp,x,'double');
fclose(fp);
cmd_convert = [trans_format_matlab_petsc_prog, ' ', vec_file_mat, ' ', vec_file_petsc, ' ', num2str(length(x))];
[status, result] = system(cmd_convert);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long int PetscInt;

#define LITTLE_ENDIAN_SYS   0
#define BIG_ENDIAN_SYS      1

#define DoByteSwap(x) ByteSwap((unsigned char *) &x, sizeof(x))

int machineEndianness(){
   long int i = 1;
   const char *p = (const char *) &i;
	/* check if lowest address contains the least significant byte */
   if (p[0] == 1)  
      return LITTLE_ENDIAN_SYS;
   else
      return BIG_ENDIAN_SYS;
}


void ByteSwap(unsigned char * b, int n){
   register int i = 0;
   register int j = n-1;
   unsigned char temp;
   while (i<j){
      /* swap(b[i], b[j]) */
	  temp = b[i];
	  b[i] = b[j];
	  b[j] = temp;
      i++, j--;
   }
}


int main(int argc, char *argv[]){
    int i, j, num_rows;
    size_t one = 1;
    FILE *io;
    double *vector_vals;
    char filename_in[1000];
    char filename_out[1000];
    char num_rows_str[1000];

    int mySystemType = machineEndianness();
    PetscInt VEC_FILE_COOKIE = 1211214;
    PetscInt *num_rows_b, *VEC_FILE_COOKIE_b;
    double *vector_val_b;

    printf("running Matlab to Petsc conversion code\n");

    /* check setup command line arguments */
    if(argc != 4){
        printf("three command line args required: input and output filenames and number of rows\n");
        return 1;
    }
    strcpy(filename_in,argv[1]);
    strcpy(filename_out,argv[2]);
    num_rows = atoi(argv[3]);

    printf("file in: %s\n", filename_in);
    printf("file out: %s\n", filename_out);
    printf("num_rows: %d\n", num_rows);


    /* setup space */
    vector_vals = (double*)malloc(num_rows*sizeof(double));

    /* read matlab binary file */
    printf("reading matlab file: %s\n", filename_in);
    io = fopen(filename_in,"r");
    for(i=0; i<num_rows; i++){
        fread(vector_vals+i,sizeof(double),one,io); 
    }
    fclose(io);

    /* write PETSc binary file */
    printf("writing PETSc file: %s\n", filename_out);
    if( mySystemType == BIG_ENDIAN_SYS ){
     	printf("Detected big-endian system. No byte swapping will be used.\n");

	io = fopen(filename_out,"w");

	fwrite (&VEC_FILE_COOKIE,sizeof(int),one,io);   

	fwrite (&num_rows,sizeof(int),one,io); 
	
        fwrite (vector_vals, sizeof(double),(size_t) num_rows, io); /* actual data */
	
        fclose(io);
  
    } 
    else {
        printf("Detected little-endian system. Byte swapping will be used.\n");

	io = fopen(filename_out,"w");
	
	VEC_FILE_COOKIE_b = (PetscInt *)malloc(sizeof(PetscInt));
	*VEC_FILE_COOKIE_b = VEC_FILE_COOKIE;
	DoByteSwap(VEC_FILE_COOKIE_b[0]);
	fwrite (VEC_FILE_COOKIE_b,sizeof(PetscInt),one,io);   


        num_rows_b = (PetscInt *)malloc(sizeof(PetscInt));
	*num_rows_b = (PetscInt)num_rows;
	DoByteSwap(num_rows_b[0]);
	fwrite (num_rows_b,sizeof(PetscInt),one,io); 


	/* actual data */
	vector_val_b = (double *)malloc(sizeof(double));	
	for (j=0; j<num_rows; j++) {
	    *vector_val_b = vector_vals[j];
	    DoByteSwap(vector_val_b[0]);
    	    fwrite (vector_val_b,sizeof(double),one,io);
	}
	fclose(io);

        /* free stuff */
        free(VEC_FILE_COOKIE_b);
        free(num_rows_b);
        free(vector_val_b);
 }

    return 0;
}

