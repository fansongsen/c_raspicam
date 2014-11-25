#ifndef M_COMPONENTS_H
#define M_COMPONENTS_H

#include "interface/mmal/mmal.h"

void set_port_default_format(MMAL_ES_FORMAT_T *port_fmt);
MMAL_STATUS_T set_preview_component_defaults(MMAL_COMPONENT_T *preview);


#endif
