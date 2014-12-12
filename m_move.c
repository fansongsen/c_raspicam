#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "m_move.h"
#include "m_file.h"
#include "m_options.h"

//#define THRESHOLD 50

#define MAX_NAME_SIZE 64

static unsigned offset=0;


//void open_new_file_handle(USERDATA_T *callback, char* name, unsigned name_size, unsigned num)
void open_new_file_handle(USERDATA_T *callback, char* name)
{
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(output_file_name, MAX_NAME_SIZE, "%d%d%d_%d%d%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	//char txt[64]="";
	//snprintf(txt, name_size+6, "%s%04d.h264", name, num);
	
	//vcos_assert((callback->file_handle = fopen(txt, "wb")) != NULL);
	vcos_assert((callback->file_handle = fopen(output_file_name, "wb")) != NULL);
}

void open_file_if_closed(USERDATA_T *callback)
{
}

void close_file_if_opened(USERDATA_T *callback)
{
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
        fprintf(stderr,"Updated tmp buffer\n");
        offset = 0;
}


unsigned movement_detected(MMAL_BUFFER_HEADER_T *buffer, void* buff_tmp_data) {
	static unsigned moved = 0;
        unsigned counter = buffer->length; // each run it will test count_per_run pixels
	if (moved == 1)
		update_tmp_buff(buffer->data, buff_tmp_data, buffer->length);
	if (moved == 0)
	{
		for (; counter > 0 && (offset < buffer->length); counter -=5,offset+=5){
			if (abs(*((char*)buff_tmp_data + offset) - *((char*)buffer->data + offset)) > threshold) 
			{
				//TODO close current file if any opened
				//TODO set name for file from date & time and open it
				moved = 30;
				update_tmp_buff(buffer->data, buff_tmp_data, buffer->length);
				break;
			}
		}    
	}
        if (offset >= buffer->length)
                offset=0;
	moved = moved == 0 ? 0 : moved-1;
	return moved;
}

