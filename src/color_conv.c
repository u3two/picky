#include "color_conv.h"

#include <math.h>

void hsl_to_rgb(int32_t hue, double saturation, double value,
		int32_t *red, int32_t *green, int32_t *blue)
{
    double chroma = value * saturation;
    double x = chroma * (1 - fabs(fmod(hue / 60.f, 2) - 1));
    double m = value - chroma;

    double r1, g1, b1;

    if (hue >= 0 && hue < 60) {
	r1 = chroma; g1 = x; b1 = 0;
    } else if (hue >= 60 && hue < 120) {
	r1 = x; g1 = chroma; b1 = 0;
    } else if (hue >= 120 && hue < 180) {
	r1 = 0; g1 = chroma; b1 = x;
    } else if (hue >= 180 && hue < 240) {
	r1 = 0; g1 = x; b1 = chroma;
    } else if (hue >= 240 && hue < 300) {
	r1 = x; g1 = 0; b1 = chroma;
    } else {
	r1 = chroma; g1 = 0; b1 = x;
    }

    *red = (r1 + m) * 255;
    *green = (g1 + m) * 255;
    *blue = (b1 + m) * 255;
}
