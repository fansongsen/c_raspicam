#ifndef M_GRAPHICS_H
#define M_GRAPHICS_H

#include "bcm_host.h"

/* Raspberry Pi requires that the bcm_host_init() 
   function is called first before any GPU calls can be made. 
*/

void init_graphics();


#endif
