/*
read PETSC binary vector and get its size from the file

compile with
gcc -o get_petsc_vector_length get_petsc_vector_length.c 
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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


PetscInt getVectorSize(char * filename);


int main ( int argc, char *argv[] )
{
    char filename[200];
    PetscInt vec_size;

    /* copy command line argument */
    if( argc < 2 ){
        printf("Need filename for PETSc vector\n");
        return;
    }
    strcpy(filename,argv[1]);
    
    /* get the vector from the binary file */ 
    vec_size = getVectorSize(filename);

    /* print and exit */
    printf("%ld\n", vec_size);
    return 0;
}


/* gets vec size from a binary file */ 
PetscInt getVectorSize(char * filename){

    int  i,j,int_val;
    FILE *io;
    size_t one = 1;
    PetscInt num_rows_return_val = 0;

    /* normal values */
    PetscInt *VEC_FILE_COOKIE, *num_rows;
    double *data_val, *data_values;
        
    /* byte swapped values */
    PetscInt *VEC_FILE_COOKIE_b, *num_rows_b;
    double *data_val_b, *data_values_b;


    int mySystemType = machineEndianness();
    int buflen, status;

    /* initialize space */
    VEC_FILE_COOKIE = (PetscInt*)malloc(sizeof(PetscInt));
    VEC_FILE_COOKIE_b = (PetscInt*)malloc(sizeof(PetscInt));
    num_rows = (PetscInt*)malloc(sizeof(PetscInt));
    num_rows_b = (PetscInt*)malloc(sizeof(PetscInt));

    if( mySystemType == BIG_ENDIAN_SYS ){

	io = fopen(filename,"r");

	fread (VEC_FILE_COOKIE,sizeof(PetscInt),one,io);   

	fread (num_rows,sizeof(PetscInt),one,io); 
	
        fclose(io);

  } else {

	io = fopen(filename,"r");

	fread (VEC_FILE_COOKIE_b,sizeof(PetscInt),one,io);   
	*VEC_FILE_COOKIE = *VEC_FILE_COOKIE_b;
	DoByteSwap(VEC_FILE_COOKIE[0]);

	fread (num_rows_b,sizeof(PetscInt),one,io); 
	*num_rows = *num_rows_b;
	DoByteSwap(num_rows[0]);

	fclose(io);
 }

    num_rows_return_val = num_rows[0];
    free(VEC_FILE_COOKIE);
    free(VEC_FILE_COOKIE_b);
    free(num_rows);
    free(num_rows_b);

    return num_rows_return_val;
}

