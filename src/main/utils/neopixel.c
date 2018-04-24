#include "neopixel.h"
#include <stdlib.h>
#include "services/io_controller.h"

#define _COLOR_SHIFT		(8)
#define _COLOR_MASK			(0x03)
#define _ADDRESS_SHIFT		(10)
#define _ADDRESS_MASK		(0x3F)


#define _BUILD_PACKET(address, color, intensity) ((intensity) | ((color & _COLOR_MASK) << (_COLOR_SHIFT)) | ((address & _ADDRESS_MASK) << (_ADDRESS_SHIFT)))

void _PublishChange(neopixel_t* neopixel)
{
	uint16_t write_data = _BUILD_PACKET(neopixel->pixel_address, NEOPIXEL_COLOR_RED, ((neopixel->rgb>>16) & 0xff));
	IoController_Write(neopixel->register_address, write_data);

	write_data = _BUILD_PACKET(neopixel->pixel_address, NEOPIXEL_COLOR_GREEN, ((neopixel->rgb>>8) & 0xff));
	IoController_Write(neopixel->register_address, write_data);

	write_data = _BUILD_PACKET(neopixel->pixel_address, NEOPIXEL_COLOR_BLUE, (neopixel->rgb & 0xff));
	IoController_Write(neopixel->register_address, write_data);
}

void Neopixel_Create(uint8_t register_address, uint8_t pixel_address, uint32_t rgb, neopixel_t* neopixel)
{
	neopixel->register_address = register_address;
	neopixel->pixel_address = pixel_address;
	neopixel->rgb = rgb;

	_PublishChange(neopixel);
}

void Neopixel_SetRgb(neopixel_t* neopixel, uint32_t rgb) {
	neopixel->rgb = rgb;
	_PublishChange(neopixel);
}