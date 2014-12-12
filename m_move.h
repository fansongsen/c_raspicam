#ifndef M_MOVE_H
#define M_MOVE_H

#include "interface/mmal/mmal.h"
void* init_tmp_buffer(const void* data, int frame_sz);
void update_tmp_buff(const void* data, void* buff, unsigned frame_sz);
unsigned movement_detected(MMAL_BUFFER_HEADER_T *buffer, void* buff_tmp_data);

#endif
