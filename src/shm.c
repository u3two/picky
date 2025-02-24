#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

int allocate_shm_file(size_t size)
{
    int fd = memfd_create("surf", 0);
    if (fd < 0)
	return -1;

    int ret;
    do {
	ret = ftruncate(fd, size);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0) {
	close(fd);
	return -1;
    }

    return fd;
}
