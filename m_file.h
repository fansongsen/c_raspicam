#ifndef M_FILE_H
#define M_FILE_H

#include "interface/mmal/mmal.h"

typedef struct
{
	FILE *file_handle;        
	MMAL_POOL_T *pool; 
} USERDATA_T;

#endif
