#define WIDTH 1920
#define HEIGHT 1080

#include "m_components.h"

// definition of MMAL_COMPONENT_DEFAULT_CAMERA
#include "util/mmal_default_components.h"

void set_port_default_format(MMAL_ES_FORMAT_T *port_fmt){
	port_fmt->encoding_variant 	= MMAL_ENCODING_I420;
	port_fmt->encoding 		= MMAL_ENCODING_OPAQUE;
	port_fmt->es->video.width 	= VCOS_ALIGN_UP(WIDTH, 32);
	port_fmt->es->video.height 	= VCOS_ALIGN_UP(HEIGHT, 16);
	port_fmt->es->video.crop.x 	= 0;
	port_fmt->es->video.crop.y 	= 0;
	port_fmt->es->video.crop.width 	= WIDTH;
	port_fmt->es->video.crop.height = HEIGHT;
	port_fmt->es->video.frame_rate.num = 0;
}

/***
 * Preview:
 ***/
MMAL_STATUS_T set_preview_component_defaults(MMAL_COMPONENT_T *preview){
	MMAL_DISPLAYREGION_T param;
	param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
	param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);

	param.set = MMAL_DISPLAY_SET_LAYER;
	param.layer = 2;
	param.set |= MMAL_DISPLAY_SET_ALPHA;
	param.alpha = 255;
	param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
	param.fullscreen = 1;
	return mmal_port_parameter_set(preview->input[0], &param.hdr);
}
