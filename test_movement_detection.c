#include <stdio.h>
#include <unistd.h>
#include <sys/time.h> // using the timer to know how many frames/sec

#include "interface/mmal/mmal.h"
#include "util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

// vc framework includes
#include "interface/vcos/vcos_assert.h"

#include "m_file.h"
#include "m_components.h"
#include "m_move.h"
#include "m_options.h"

#define MAX_OUTPUTED_FRAMES 300

void consume_queue_on_connection(MMAL_PORT_T *port, MMAL_QUEUE_T *queue);
void consume_queue_on_connection_if_moves(MMAL_PORT_T *port, MMAL_QUEUE_T *queue);

void* buff_tmp_data = 0;
int exit_prog = 0;
static unsigned sent_frames= 0;
static struct timeval tm1={0};

static unsigned calibrating = 1;

static inline void start()
{
    gettimeofday(&tm1, NULL);
}

static inline double elapsed(){
    struct timeval tm3={0};
    gettimeofday(&tm3, NULL);
    double elapsed = (1000 * (tm3.tv_sec - tm1.tv_sec) + (tm3.tv_usec - tm1.tv_usec) / 1000);
    //fprintf(stderr, "Time elapsed: tm3:%ld,%ld tm1:%ld,%ld  %f\n", tm3.tv_sec, tm3.tv_usec, tm1.tv_sec, tm1.tv_usec, elapsed);
    return elapsed;
}

static inline void stop()
{
    struct timeval tm2;
    gettimeofday(&tm2, NULL);

    unsigned long long t = 1000 * (tm2.tv_sec - tm1.tv_sec) + (tm2.tv_usec - tm1.tv_usec) / 1000;
    printf("%llu ms elapsed, %u frames, frequency: %f f/s\n", t, sent_frames, sent_frames*1000.00/t);
}

void connection_video2encoder_callback(MMAL_CONNECTION_T *conn)
{
	consume_queue_on_connection(conn->out, conn->pool->queue);
	consume_queue_on_connection_if_moves(conn->in, conn->queue);
}

void send_buff_to_encoder(MMAL_PORT_T *port, MMAL_QUEUE_T *queue, MMAL_BUFFER_HEADER_T *buffer)
{
	if (mmal_port_send_buffer(port, buffer) != MMAL_SUCCESS)
	{
		mmal_queue_put_back(queue, buffer);
	}
}

void consume_queue_on_connection_if_moves(MMAL_PORT_T *port, MMAL_QUEUE_T *queue)
{
	MMAL_BUFFER_HEADER_T *buffer;
	static moving = 0;
	
	while ((buffer = mmal_queue_get(queue)) != NULL)
	{
		// if the frame is not initialized, init
		if (!buff_tmp_data)
			buff_tmp_data = init_tmp_buffer(buffer->data, buffer->length);

		if (calibrating) {
			if (tm1.tv_usec == 0 && tm1.tv_sec == 0) { 
				start(); // start calibration time
				fprintf(stderr, "Calibration starts\n");
			} else if (elapsed() > 3000) {   
				fprintf(stderr, "Calibration finished\n");
				calibrating =0;
				tm1.tv_sec  =0;
				tm1.tv_usec =0;
				// get a snapshot
				update_tmp_buff(buffer->data, buff_tmp_data, buffer->length);
			}
			send_buff_to_encoder(port, queue, buffer);	// during calibration, video is recorded

		} else if (!moving) { // calibration over, waiting for move
			if (movement_detected(buffer, buff_tmp_data)) {
				fprintf(stderr, "Recording starts\n");
				moving = 1;
				start(); 
				send_buff_to_encoder(port, queue, buffer);	// during calibration, video is recorded
			} else {
        			mmal_buffer_header_release(buffer);		// nothing moved, discard img
			}

		} else {	// something is moving, save the next n seconds
			send_buff_to_encoder(port, queue, buffer);		// save action
			if (!exit_prog && elapsed() > 5000) {
				exit_prog=1;
				fprintf(stderr,"Stopped!\n");
				stop();
			}
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


int main(int argc, char **argv)
{
	MMAL_COMPONENT_T *camera =0;
	MMAL_COMPONENT_T *preview=0;
	MMAL_COMPONENT_T *encoder=0;
	MMAL_CONNECTION_T *preview_conn=0;
	MMAL_CONNECTION_T *encoder_conn=0;
	USERDATA_T callback_data;

	// bcm should be initialized before any GPU call is made
	bcm_host_init();

	setNonDefaultOptions(argc, argv);

	// Open file to save the video 
	open_new_file_handle(&callback_data, output_file_name);
	fprintf(stderr, "Output Filename: %s\n", output_file_name);

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

	while(1)
	{
        	vcos_sleep(1000); // wait for exit call	
		if (exit_prog){
        		vcos_sleep(500); // exit called, wait for all buffers be flushed
			break;
		}
	}

	return 0;
}
