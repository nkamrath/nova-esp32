#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	NEOPIXEL_COLOR_RED,
	NEOPIXEL_COLOR_GREEN,
	NEOPIXEL_COLOR_BLUE
} neopixel_color_t;

typedef struct
{
	uint8_t register_address; //register address for writing to this pixel
	uint8_t pixel_address;	  //pixel address for writing to this pixel (index within the neopixel chain)
	uint32_t rgb;
} neopixel_t;

void Neopixel_Create(uint8_t register_address, uint8_t pixel_address, uint32_t rgb, neopixel_t* neopixel);
void Neopixel_SetRgb(neopixel_t* neopixel, uint32_t rgb);

#endif