#include "args.h"
#include "config.h"
#include "macros.h"

#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

OutputFormat parse_outformat(const char *str)
{
    if (!strcmp(str, "hex")) return OUTFORMAT_HEX;
    else if (!strcmp(str, "rgb")) return OUTFORMAT_RGB;
    else if (!strcmp(str, "hsv")) return OUTFORMAT_HSV;
    return OUTFORMAT_INVALID;
}

// Parses an _unsigned_ integer from [str]. Returns -1 if no valid number is found, overflow or other error.
int parse_uint(const char *str)
{
    errno = 0;
    char *endptr;
    unsigned long res = strtoul(str, &endptr, 10);
    if (endptr == str || errno || res > INT_MAX)
	return -1;
    return res;
}

void print_usage(const char *progname)
{
    printf("\
usage: %s [OPTIONS]							\n\
Options:								\n\
  --help                    show this message				\n\
  -w --width  [PIXELS]      set window width (default: "STRINGIFY_EXP(PICKY_DEFAULT_WIDTH)")\n\
  -h --height [PIXELS]      set window height (default: "STRINGIFY_EXP(PICKY_DEFAULT_HEIGHT)")\n\
  -z --zoom-region [PIXELS] set the zoom region size (default: "STRINGIFY_EXP(PICKY_REGION_SIZE)")\n\
  -b --zoom-border [PIXELS] set the zoom border size (default: "STRINGIFY_EXP(PICKY_BORDER_SIZE)")\n\
  -u --hue [DEG]            set hue (in HSV degrees) (default: "STRINGIFY_EXP(PICKY_INITIAL_HUE)")\n\
  -l --lock-hue             lock the hue (disable scroll)		\n\
  -f --format [hex|rgb|hsv] set the output format (CSS-ish style)	\n\
",
	   progname);
}

void args_parse(Args *args, int argc, char **argv)
{
    // initialize defaults
    *args = (Args) {
	.win_width = PICKY_DEFAULT_WIDTH,
	.win_height = PICKY_DEFAULT_HEIGHT,
	.zoom_region_size = PICKY_REGION_SIZE,
	.zoom_border_size = PICKY_BORDER_SIZE,
	.lock_hue = false,
	.initial_hue = PICKY_INITIAL_HUE,
	.output_format = PICKY_DEFAULT_FORMAT,
    };

    #define ARG_ENSURE_NEXT(name) do {			  \
	if (!argv[i+1])	{				  \
	    fprintf(stderr, name" needs an argument, maybe try --help?\n"); \
	    exit(1);					  \
        }						  \
    } while (0)

    #define ARG_PARSE_INT(name, into) do {			\
        i++;							\
	int res = parse_uint(argv[i]);				\
	if (res == -1) {					\
	    fprintf(stderr, name": failed to parse number, maybe try --help?\n"); \
	    exit(1);						\
        }							\
    into = res;							\
    } while(0)

    for (int i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "--help")) {
	    print_usage(argv[0]);
	    exit(0);
	} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--height")) {
	    ARG_ENSURE_NEXT("height");
	    ARG_PARSE_INT("height", args->win_height);
	} else if (!strcmp(argv[i], "-w") || !strcmp(argv[i], "--width")) {
	    ARG_ENSURE_NEXT("width");
	    ARG_PARSE_INT("width", args->win_width);
	} else if (!strcmp(argv[i], "-z") || !strcmp(argv[i], "--zoom-region")) {
	    ARG_ENSURE_NEXT("zoom region");
	    ARG_PARSE_INT("zoom region", args->zoom_region_size);
	} else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--zoom-border")) {
	    ARG_ENSURE_NEXT("zoom border size");
	    ARG_PARSE_INT("zoom border size", args->zoom_border_size);
	} else if (!strcmp(argv[i], "-u") || !strcmp(argv[i], "--hue")) {
	    ARG_ENSURE_NEXT("hue");
	    ARG_PARSE_INT("hue", args->initial_hue);
	    args->initial_hue %= 360;
	} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--lock-hue")) {
	    args->lock_hue = true;
	} else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--format")) {
	    ARG_ENSURE_NEXT("format");
	    args->output_format = parse_outformat(argv[i+1]);
	    if (args->output_format == OUTFORMAT_INVALID) {
		fprintf(stderr, "invalid format '%s', maybe try --help?\n", argv[i]);
		exit(1);
	    }
	    i++;
	} else {
	    fprintf(stderr, "unrecognized argument '%s', maybe try --help?\n", argv[i]);
	    exit(1);
	}
    }
}
