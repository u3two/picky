#ifndef PICKY_ARGS_H
#define PICKY_ARGS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    OUTFORMAT_HEX,
    OUTFORMAT_RGB,
    OUTFORMAT_HSV,
    OUTFORMAT_INVALID,
} OutputFormat;

typedef struct {
    int32_t win_width, win_height;
    int32_t zoom_region_size, zoom_border_size;
    int32_t initial_hue;
    bool lock_hue;
    OutputFormat output_format;
} Args;

void args_parse(Args *args, int argc, char **argv);

#endif /* PICKY_ARGS_H */
