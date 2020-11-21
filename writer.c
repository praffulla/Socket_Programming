#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main () {

    int shm_fd;
    shm_fd = shm_open("/intro", O_CREAT | O_RDWR, 0660);
    if (shm_fd < 0) {
        perror("shm_open failed");
        exit(1);
    }

    ftruncate(shm_fd, 1024);
    
    void *shm_rgn = mmap( NULL,
            1024,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            shm_fd,
            0        
    );

    memset(shm_rgn, 0, 1024);
    memcpy(shm_rgn, "Hello !, This is a demo program!",
                     strlen("Hello !, This is a demo program!"));
    munmap(shm_rgn, 1024);
    close(shm_fd);
    return 0;
}

