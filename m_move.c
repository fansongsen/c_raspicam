#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "m_move.h"
#include "m_file.h"
#include "m_options.h"

//#define MAX_NAME_SIZE 64

void open_new_file_handle(USERDATA_T *callback, char* name)
{
	vcos_assert((callback->file_handle = fopen(output_file_name, "wb")) != NULL);
}

void* init_tmp_buffer(const void* data, int frame_sz){
        void* new_buff = (void *)malloc(frame_sz);
        if (new_buff == NULL) exit(1);
        update_tmp_buff(data, new_buff, frame_sz);
        fprintf(stderr,"Initializing buffer with size %u\n", frame_sz);
        return new_buff;    
}

void update_tmp_buff(const void* data, void* buff, unsigned frame_sz){
        memcpy(buff, data, frame_sz);
}

/*Compare images, uses the green channel and compares every 10 pixels*/
unsigned movement_detected(MMAL_BUFFER_HEADER_T *buffer, void* buff_tmp_data) {
	unsigned moved=0;
        unsigned counter = buffer->length; 

	char *a=(char*)buff_tmp_data+buffer->length-1;
	char *b=(char*)buffer->data+buffer->length-1;
	char *buff=(char*)buff_tmp_data;
	for (; a > buff ; a-=30, b-=30){
		if (abs((*a) - (*b)) > threshold) 
		{
			fprintf(stderr,"Movement detected - %d - %d = %d\n", *a, *b, *a - *b);
			moved = 1;
			break;
		}
	}    
	update_tmp_buff(buffer->data, buff_tmp_data, buffer->length);
	return moved;
}

