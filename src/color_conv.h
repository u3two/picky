#ifndef PICKY_COLOR_CONV_H
#define PICKY_COLOR_CONV_H

#include <stdint.h>

void hsl_to_rgb(int32_t hue, double saturation, double value,
		int32_t *red, int32_t *green, int32_t *blue);

#endif
