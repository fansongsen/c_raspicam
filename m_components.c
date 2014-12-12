#define WIDTH 1920
#define HEIGHT 1080

#include "m_components.h"

/*
Notes:
	I am not sure if this set_camera_port* functions are
strictly necessary. 
I have some issues with the speed of the video playback, and
was clear that if I used MMAL_ENCODING_OPAQUE the issue went away.
The problem is that with OPAQUE, just the headers are visible,
and therefore I cant compare the pixels in memory.

TODO: Some possible solutions 
[1]: see if you can access the pixels with MMAL_ENCODING_OPAQUE
[2]: see if you can handle the problems with motion vectors. 
They should come together with frames and based on them we could 
decide whether to drop the buffer or send it to encoder
[3]: Create a component instead doing this by hand.
MMAL lets user define components. 
Using tunneling between components should improve performance,
the question is if doing this inside a component would improve it.

[1] and [2] are cleaner, but if the [3] works... why not?
*/

void set_camera_port1_format(MMAL_ES_FORMAT_T *port_fmt)
{
	port_fmt->es->video.frame_rate.num = 30;
}

void set_port_default_format(MMAL_ES_FORMAT_T *port_fmt){
	//port_fmt->encoding 		= MMAL_ENCODING_OPAQUE;
	port_fmt->encoding 		= MMAL_ENCODING_I420;
	port_fmt->encoding_variant 	= MMAL_ENCODING_I420;
	port_fmt->es->video.width 	= VCOS_ALIGN_UP(WIDTH, 32);
	port_fmt->es->video.height 	= VCOS_ALIGN_UP(HEIGHT, 16);
	port_fmt->es->video.crop.x 	= 0;
	port_fmt->es->video.crop.y 	= 0;
	port_fmt->es->video.crop.width 	= WIDTH;
	port_fmt->es->video.crop.height = HEIGHT;
	port_fmt->es->video.frame_rate.den = 1;
	port_fmt->es->video.frame_rate.num = 0;
}

/***
 * Camera:
 ***/
MMAL_STATUS_T set_camera_component_defaults(MMAL_COMPONENT_T *camera)
{
	// port0 
	set_port_default_format(camera->output[0]->format);
      	assert((mmal_port_format_commit(camera->output[0]) == MMAL_SUCCESS)
		&& "Could not commit format camera output");

	// port1 
	set_port_default_format(camera->output[1]->format);
	set_camera_port1_format(camera->output[1]->format);
      	assert((mmal_port_format_commit(camera->output[1]) == MMAL_SUCCESS)
		&& "Could not commit format camera output");

	// port2 
	set_port_default_format(camera->output[2]->format);
      	assert((mmal_port_format_commit(camera->output[2]) == MMAL_SUCCESS)
		&& "Could not commit format camera output");

	return MMAL_SUCCESS;
}

/***
 * Preview:
 ***/
MMAL_STATUS_T set_preview_component_defaults(MMAL_COMPONENT_T *preview)
{
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

/***
 * Encoder 
 */

void set_encoder_component_defaults(MMAL_COMPONENT_T *encoder)
{
        encoder->output[0]->format->encoding = MMAL_ENCODING_H264;
	encoder->output[0]->format->es->video.frame_rate.num = 0;
	encoder->output[0]->format->es->video.frame_rate.den = 1;

        vcos_assert((mmal_port_format_commit(encoder->output[0]) == MMAL_SUCCESS)
                && "Problem trying to format the encoder output\n");

        MMAL_PARAMETER_VIDEO_PROFILE_T  param;
        param.hdr.id = MMAL_PARAMETER_PROFILE;
        param.hdr.size = sizeof(param);
        param.profile[0].profile = MMAL_VIDEO_PROFILE_H264_HIGH;
        param.profile[0].level = MMAL_VIDEO_LEVEL_H264_4; // This is the only value supported
        vcos_assert((mmal_port_parameter_set(encoder->output[0], &param.hdr) == MMAL_SUCCESS) 
		&& "Failed setting port parameters");
}

