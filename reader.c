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
    char buffer[1024] = {0};
    shm_fd = shm_open("/intro", O_CREAT | O_RDONLY, 0660);
    if (shm_fd < 0) {
        perror("shm_open failed in reader");
        exit(1);
    }

    void *shm_reg =  mmap(NULL, 1024, PROT_READ, MAP_SHARED, shm_fd, 0);
    
    if (shm_reg == MAP_FAILED) {
        perror("mmap from Kernel to VAS failed");
        exit(1);
    }

    memcpy(buffer, shm_reg, 1024);
    
    munmap(shm_reg, 1024);

    printf("Got the data from writer: %s\n", buffer);
    close(shm_fd);
    shm_unlink("/intro");

    return 0;
}

