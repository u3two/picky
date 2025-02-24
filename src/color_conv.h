#ifndef PICKY_COLOR_CONV_H
#define PICKY_COLOR_CONV_H

#include <stdint.h>
#include "appstate.h"

void hsl_to_rgb(int32_t hue, double saturation, double value,
		int32_t *red, int32_t *green, int32_t *blue);

void xy_to_rgb(AppState *state, int32_t x, int y, int *red, int *green, int *blue);

#endif
