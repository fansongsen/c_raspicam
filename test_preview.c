#include <stdio.h>

#include "interface/mmal/mmal.h"
#include "util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

// vc framework includes
#include "interface/vcos/vcos_assert.h"

#include "m_video.h"
#include "m_graphics.h"
#include "m_components.h"

void main(void)
{
	MMAL_COMPONENT_T *camera =0;
	MMAL_COMPONENT_T *preview=0;
	MMAL_CONNECTION_T *preview_conn=0;
	
	// bcm should be initialized before any GPU call is made
	bcm_host_init();

	vcos_assert(( init_state() == 0) && "Checking setting initial state failed");

	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera) == MMAL_SUCCESS) && "Failed creating camera component");

	vcos_assert((mmal_port_enable(camera->control, 0) == MMAL_SUCCESS) && "Failed enabling camera control port");

	set_port_default_format(camera->output[0]->format);
	vcos_assert((mmal_port_format_commit(camera->output[0]) == MMAL_SUCCESS) && "Failed setting preview port format");
	
	vcos_assert((mmal_component_enable(camera) == MMAL_SUCCESS) && "Failed to enable camera component");

	vcos_assert((mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &preview) == MMAL_SUCCESS) && "Failed creating preview component");
	vcos_assert(( set_preview_component_defaults(preview) == MMAL_SUCCESS) && "Failed setting preview components default values");

	vcos_assert((mmal_component_enable(preview) == MMAL_SUCCESS) && "Failed to enable camera component");

	vcos_assert((mmal_connection_create(&preview_conn, camera->output[0], preview->input[0], 
		MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS) 
		&& "Failed stablishing connection");

	vcos_assert((mmal_connection_enable(preview_conn) == MMAL_SUCCESS) && "Failed enabling connection");

	while (1)
		mmal_port_parameter_set_boolean(camera->output[1], MMAL_PARAMETER_CAPTURE, 1);
}
