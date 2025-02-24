#ifndef PICKY_SHM_H
#define PICKY_SHM_H

#include <sys/mman.h>

int allocate_shm_file(size_t size);

#endif
