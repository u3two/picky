#ifndef PICKY_MACROS_H
#define PICKY_MACROS_H

#include <stdio.h>
#include <stdlib.h>

#define FATAL(...)				\
    do {					\
	fprintf(stderr,				\
		"["__FILE__			\
		":%d] ",			\
		__LINE__);			\
	fprintf(stderr, "FATAL: "__VA_ARGS__);	\
	fputc('\n', stderr);			\
	exit(1);				\
    } while (0)

#ifdef DEBUG_ENABLE
#define DEBUG(...)				\
    do {					\
	fprintf(stderr,				\
		"["__FILE__			\
		":%d] ",			\
		__LINE__);			\
	fprintf(stderr, "DEBUG: "__VA_ARGS__);	\
	fputc('\n', stderr);			\
    } while (0)
#else
#define DEBUG(...) /* NOP */
#endif

#define MAX(a,b)				\
    ({ __typeof__ (a) _a = (a);			\
	__typeof__ (b) _b = (b);		\
	_a > _b ? _a : _b; })

#define MIN(a,b)				\
    ({ __typeof__ (a) _a = (a);			\
	__typeof__ (b) _b = (b);		\
	_a < _b ? _a : _b; })

#define STRINGIFY(x) #x
#define STRINGIFY_EXP(x) STRINGIFY(x)

#endif /* PICKY_MACROS_H */
