#include <stdio.h>
#include <unistd.h>

#include "interface/mmal/mmal.h"
#include "util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

// vc framework includes
#include "interface/vcos/vcos_assert.h"

#include "m_video.h"
#include "m_components.h"

#define RUN_DURATION 10 // every how many frames we refresh the buffer
#define THRESHOLD 50

typedef struct
{
	FILE *file_handle;        
	MMAL_POOL_T *pool; 
} USERDATA_T;

void consume_queue_on_connection(MMAL_PORT_T *port, MMAL_QUEUE_T *queue);
void consume_queue_on_connection_if_moves(MMAL_PORT_T *port, MMAL_QUEUE_T *queue);
void update_tmp_buff(const void* data, void* buff, unsigned frame_sz);

static void* buff_tmp_data = 0;
unsigned offset=0;

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

unsigned moved(const void* frame_data, void* buff_tmp_data, int frame_sz) {
	static unsigned if_moved = 0;
        unsigned count_per_run = frame_sz/(RUN_DURATION); // each run it will test count_per_run pixels
	if (if_moved == 0)
	{
		for (; count_per_run > 0 && (offset < frame_sz); count_per_run--,offset++){
			if (abs(*((char*)buff_tmp_data + offset) - *((char*)frame_data + offset)) > THRESHOLD) 
			{
				if_moved = 10;
				update_tmp_buff(frame_data, buff_tmp_data, frame_sz);
				break;
			}
		}    
	}
        if (offset >= frame_sz)
                update_tmp_buff((void *)frame_data, (void *)buff_tmp_data, frame_sz);
	if_moved = if_moved == 0 ? 0 : if_moved-1;
	return if_moved;
}

void connection_video2encoder_callback(MMAL_CONNECTION_T *conn)
{
	consume_queue_on_connection(conn->out, conn->pool->queue);
	consume_queue_on_connection_if_moves(conn->in, conn->queue);
}

void consume_queue_on_connection_if_moves(MMAL_PORT_T *port, MMAL_QUEUE_T *queue)
{
	MMAL_BUFFER_HEADER_T *buffer;
	
	while ((buffer = mmal_queue_get(queue)) != NULL)
	{
		if (!buff_tmp_data)
			buff_tmp_data = init_tmp_buffer(buffer->data, buffer->length);
		if (moved(buffer->data, buff_tmp_data, buffer->length))
		{
			if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS)
			{
				mmal_queue_put_back(queue, buffer);
			}
		}
		else
		{
        		mmal_buffer_header_release(buffer);
		}
	}
}
void consume_queue_on_connection(MMAL_PORT_T *port, MMAL_QUEUE_T *queue)
{
	MMAL_BUFFER_HEADER_T *buffer;
	
	while ((buffer = mmal_queue_get(queue)) != NULL)
	{
			if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS)
			{
				mmal_queue_put_back(queue, buffer);
				break;
			}
	}
}

static void encoder_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
        USERDATA_T *data;

        vcos_assert((data = (USERDATA_T *)port->userdata) != NULL
			&& "No data associated with this buffer");

        vcos_assert((buffer && buffer->length > 0)
		&& "buffer == NULL or buffer has size 0");

	mmal_buffer_header_mem_lock(buffer);
	vcos_assert((fwrite(buffer->data, 1, buffer->length, data->file_handle) == buffer->length) 
		&& "Failed writing buffer to file (different size returned)");
	mmal_buffer_header_mem_unlock(buffer);

        mmal_buffer_header_release(buffer);

	// send one buffer back
        if (port->is_enabled)
        {
                MMAL_BUFFER_HEADER_T *new_buffer;
                if ((new_buffer = mmal_queue_get(data->pool->queue)) != NULL )
                        vcos_assert((mmal_port_send_buffer(port, new_buffer) == MMAL_SUCCESS)
                                && "Unable to return buffer to encoder port");
        }
}


int main(void)
{
	MMAL_COMPONENT_T *camera =0;
	MMAL_COMPONENT_T *preview=0;
	MMAL_COMPONENT_T *encoder=0;
	MMAL_CONNECTION_T *preview_conn=0;
	MMAL_CONNECTION_T *encoder_conn=0;
	USERDATA_T callback_data;

	// bcm should be initialized before any GPU call is made
	bcm_host_init();

	vcos_assert(( init_state() == 0) 
		&& "Checking setting initial state failed");

	// Open file to save the video #TODO: do not use this variable hard coded
	vcos_assert((callback_data.file_handle = fopen("/home/pi/output.h264", "wb")) != NULL);

	// Create Camera, set default values and enable it
	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera) == MMAL_SUCCESS) 
		&& "Failed creating camera component");

	vcos_assert((set_camera_component_defaults(camera)) == MMAL_SUCCESS
		&& "Failed setting camera components default values");

	vcos_assert((mmal_component_enable(camera) == MMAL_SUCCESS) 
		&& "Failed to enable camera component");

	// Create Preview, set default values and enable it
	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview) == MMAL_SUCCESS) 
		&& "Failed creating preview component");

	vcos_assert((set_preview_component_defaults(preview) == MMAL_SUCCESS) 
		&& "Failed setting preview components default values");

	vcos_assert((mmal_component_enable(preview) == MMAL_SUCCESS) 
		&& "Failed to enable camera component");

	// Create Encoder, set defaults, enable it and create encoder pool
        vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder) == MMAL_SUCCESS)
                && "Failed to create encoder component");

        vcos_assert((encoder->input_num && encoder->output_num)
                && "Video encoder does not have input/output ports\n");

        set_encoder_component_defaults(encoder);

        vcos_assert((mmal_component_enable(encoder) == MMAL_SUCCESS)
		&& "Failed enabling encoder component");

        callback_data.pool = (MMAL_POOL_T *)mmal_port_pool_create(encoder->output[0],
                        encoder->output[0]->buffer_num, encoder->output[0]->buffer_size);

        vcos_assert(callback_data.pool && "Failed creating encoder pool\n");

	// Setup connections
	vcos_assert((mmal_connection_create(&preview_conn, camera->output[0], preview->input[0], 
		MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS) 
		&& "Failed stablishing connection between camera output 0 and preview");

	vcos_assert((mmal_connection_enable(preview_conn) == MMAL_SUCCESS) 
		&& "Failed enabling connection between camera output 0 and preview");

        vcos_assert((mmal_connection_create(&encoder_conn, camera->output[1], encoder->input[0],
        //        MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS)
                MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS)
                && "Failed creating encoder connection to capture (camera output 1)");

        vcos_assert((mmal_connection_enable(encoder_conn) == MMAL_SUCCESS)
                && "Failed enabling encoder connection to capture (campera output 1)");

	encoder_conn->callback = connection_video2encoder_callback;

	// Set callback and enable encoder port
	encoder->output[0]->userdata = (struct MMAL_PORT_USERDATA_T *)&callback_data;
	vcos_assert(MMAL_SUCCESS == mmal_port_enable(encoder->output[0], encoder_callback));


	// Send the buffer to encode output port
	MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(callback_data.pool->queue);
	vcos_assert((mmal_port_send_buffer(encoder->output[0], buffer)== MMAL_SUCCESS)
		&& "Unable to send buffer to encoder output port");

	vcos_assert((mmal_port_parameter_set_boolean(camera->output[1], MMAL_PARAMETER_CAPTURE, 1) == MMAL_SUCCESS) 
		&& "Unable to set camera->output[1] capture parameter");

	consume_queue_on_connection(encoder_conn->out, encoder_conn->pool->queue);

        vcos_sleep(20000); 	// work 5 seconds and exit

	return 0;
}
