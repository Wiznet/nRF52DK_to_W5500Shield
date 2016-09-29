#ifndef USER_ETHERNET_H__
#define USER_ETHERNET_H__

#include "bsp.h"


void 	wizchip_select	(void);
void 	wizchip_deselect(void);
uint8_t wizchip_read(void);
void 	wizchip_write	(uint8_t wb);


void user_ethernet_init	(void);
void network_init		(void);

#endif
