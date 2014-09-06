/* converts a matrix in PETSc format to a matrix readable with MATLAB binary format 
gcc -o convertPetscToMatlabBinaryVec convertPetscToMatlabBinaryVec.c
Sergey Voronin

read in MATLAB via:
fp = fopen('v.bin','r');
v = fread(fp,128*128*37*6,'double');
fclose(fp);
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

    int  i,j,int_val;
    FILE *io;
    size_t one = 1;
    char filename_in[1000];
    char filename_out[1000];

    /* data */
    double *data_values;

    /* normal values */
    PetscInt *VEC_FILE_COOKIE, *num_rows;
    double *data_val;

    /* byte swapped values */
    PetscInt *VEC_FILE_COOKIE_b, *num_rows_b;
    double *data_val_b;

    int mySystemType = machineEndianness();

    /* init arguments */
    if(argc != 3){
        printf("needs 2 command line arguments: input and output filenames\n");
        return 1;
    } 
    strcpy(filename_in,argv[1]);
    strcpy(filename_out,argv[2]);
  
    /* read PETSc file to get vector of data values  ---> */
    printf("reading PETSc file %s\n", filename_in);

    /* initialize space */
    VEC_FILE_COOKIE = (PetscInt*)malloc(sizeof(PetscInt));
    VEC_FILE_COOKIE_b = (PetscInt*)malloc(sizeof(PetscInt));
    num_rows = (PetscInt*)malloc(sizeof(PetscInt));
    num_rows_b = (PetscInt*)malloc(sizeof(PetscInt));


    if( mySystemType == BIG_ENDIAN_SYS ){
	io = fopen(filename_in,"r");

	fread (VEC_FILE_COOKIE,sizeof(PetscInt),one,io);   

	fread (num_rows,sizeof(PetscInt),one,io); 
	
        data_values = (double*)malloc(sizeof(double)*(*num_rows));

        fread (data_values, sizeof(double),(size_t) num_rows, io); /* actual data */
	
        fclose(io);
    } 
    else {
	printf("Detected little-endian system. Byte swapping will be used.\n");

	io = fopen(filename_in,"r");

	fread (VEC_FILE_COOKIE_b,sizeof(PetscInt),one,io);   
	*VEC_FILE_COOKIE = *VEC_FILE_COOKIE_b;
	DoByteSwap(VEC_FILE_COOKIE[0]);

	fread (num_rows_b,sizeof(PetscInt),one,io); 
	*num_rows = *num_rows_b;
	DoByteSwap(num_rows[0]);

	
	/* actual data */
	data_values = (double *)malloc(sizeof(double)*(*num_rows));
	for (j=0; j<(*num_rows); j++) {
            data_val = (double*)malloc(sizeof(double));
            data_val_b = (double*)malloc(sizeof(double));

    	    fread(data_val_b,sizeof(double),one,io);
	    DoByteSwap(data_val_b[0]);
            *data_val = *data_val_b;

            data_values[j] = *data_val_b;

            free(data_val);
            free(data_val_b);
	}

	fclose(io);
    }

    /* write the data values we got to a binary file that can be read by matlab */
    printf("writing MATLAB file %s\n", filename_out);
    io = fopen(filename_out,"w");
    for(i=0; i<(*num_rows); i++){
        fwrite(data_values+i,sizeof(double),one,io);
    }
    fclose(io);

    return 0;
}

