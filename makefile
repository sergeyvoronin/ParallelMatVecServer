PETSC_DIR=/opt/petsc_software/petsc-3.5.1
PETSC_ARCH=arch-linux2-c-debug

include ${PETSC_DIR}/conf/variables
include ${PETSC_DIR}/conf/rules

get_petsc_vector_length: server_code/get_petsc_vector_length.o  chkopts
	-${CLINKER}  -o server_code/get_petsc_vector_length server_code/get_petsc_vector_length.o ${PETSC_VEC_LIB} 
	${RM} server_code/get_petsc_vector_length.o


petsc_server_regular: server_code/petsc_server_regular.o  chkopts
	-${CLINKER}  -o server_code/petsc_server_regular server_code/petsc_server_regular.o ${PETSC_MAT_LIB} 
	${RM} server_code/petsc_server_regular.o

