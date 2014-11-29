#include <stdio.h>

//#include <sys/types.h>
//#include <pthread.h>
//#include <unistd.h>

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_port.h" 		//internals
#include "util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

// vc framework includes
#include "interface/vcos/vcos_assert.h"

#include "m_video.h"
#include "m_components.h"
#include "bcm_host.h"

#define MMAL_PORT_TYPE_INPUT  2
#define MMAL_PORT_TYPE_OUTPUT 3

#define WIDTH 1920
#define HEIGHT 1080

void consume_queue_on_connection(MMAL_PORT_T *port, MMAL_QUEUE_T *queue);

void connection_callback(MMAL_CONNECTION_T *conn)
{
//	fprintf(stderr, "[%ld] Conn callback on conn [%p],conn in: %p,conn out: %p, Queue length: %u\n", pthread_self(), conn, conn->in, conn->out, mmal_queue_length(conn->queue));
	consume_queue_on_connection(conn->out, conn->pool->queue);
	consume_queue_on_connection(conn->in, conn->queue);
}

static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
 	fprintf(stderr,"[%ld] Camera Callback called\n", pthread_self());
   	mmal_buffer_header_release(buffer);
}

static void preview_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
 	fprintf(stderr,"[%ld] Preview Control Callback called on port %p\n", pthread_self(), port);
   	mmal_buffer_header_release(buffer);
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

int main(void)
{
	MMAL_COMPONENT_T *camera =0;
	MMAL_COMPONENT_T *preview=0;
	static MMAL_CONNECTION_T *preview_conn=0;
	MMAL_BUFFER_HEADER_T *buffer;
	void(* MMAL_PORT_BH_CB_T)(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
	
	// bcm should be initialized before any GPU call is made
	bcm_host_init();

	vcos_assert(( init_state() == 0) 
		&& "Checking setting initial state failed");

	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera) == MMAL_SUCCESS) 
		&& "Failed creating camera component");

	vcos_assert((mmal_port_enable(camera->control, camera_control_callback) == MMAL_SUCCESS) 
		&& "Failed enabling camera control port");

	set_port_default_format(camera->output[0]->format);
	vcos_assert((mmal_port_format_commit(camera->output[0]) == MMAL_SUCCESS) 
		&& "Failed setting preview port format");

	vcos_assert((mmal_component_enable(camera) == MMAL_SUCCESS) 
		&& "Failed to enable camera component");

	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview) == MMAL_SUCCESS) 
		&& "Failed creating preview component");
	
	vcos_assert((mmal_port_enable(preview->control, preview_control_callback) == MMAL_SUCCESS) 
		&& "Failed enabling preview control port");

	vcos_assert((set_preview_component_defaults(preview) == MMAL_SUCCESS) 
		&& "Failed setting preview components default values");

	vcos_assert((mmal_component_enable(preview) == MMAL_SUCCESS) 
		&& "Failed to enable camera component");

	vcos_assert((mmal_connection_create(&preview_conn, camera->output[0], preview->input[0], 
		MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS) 
		&& "Failed stablishing connection");

	vcos_assert((mmal_connection_enable(preview_conn) == MMAL_SUCCESS) 
		&& "Failed enabling connection");

	preview_conn->callback = connection_callback;

	mmal_port_parameter_set_boolean(camera->output[1], MMAL_PARAMETER_CAPTURE, 1);
	
	consume_queue_on_connection(preview_conn->out, preview_conn->pool->queue);

	vcos_sleep(5000);

	// TODO: implement finalize_state to close components and connections
	vcos_assert(( finalize_state() == 0) && "Failed freeing memory nicely" );

	fprintf(stderr, "No error, program executed just fine\n");
	return 0;
}
