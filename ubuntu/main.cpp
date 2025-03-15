#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <aio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>

#define BUFFER_SIZE 8192

struct aio_operation {
    struct aiocb aio;
    char *buffer;
    int write_operation;
};

volatile int active_writes = 0;

void aio_completion_handler(sigval_t sigval) {
    struct aio_operation *aio_op;
    ssize_t ret;
    
    aio_op = (struct aio_operation *)sigval.sival_ptr;
    if (aio_error(&aio_op->aio) == 0) {
        ret = aio_return(&aio_op->aio);
        if (ret < 0) {
            printf("AIO operation failed\n");
            exit(EXIT_FAILURE);
        }
    }
    if (aio_op->write_operation) {
        free(aio_op->buffer);
        free(aio_op);
        __sync_fetch_and_sub(&active_writes, 1);
    }
}

void setup_aio_operation(struct aio_operation *op, int fd, off_t offset, size_t size, int is_write) {
    memset(&op->aio, 0, sizeof(struct aiocb));
    op->aio.aio_fildes = fd;
    op->aio.aio_buf = op->buffer;
    op->aio.aio_nbytes = size;
    op->aio.aio_offset = offset;
    op->aio.aio_sigevent.sigev_notify = SIGEV_THREAD;
    op->aio.aio_sigevent.sigev_notify_function = aio_completion_handler;
    op->aio.aio_sigevent.sigev_value.sival_ptr = op;
}

void copy_file(const char *source, const char *destination, size_t block_size, int num_operations) {
    int src_fd, dst_fd;
    struct stat st;
    off_t file_size, offset;
    struct aiocb **aio_list;
    clock_t start, end;
    int active_ops, i;

    src_fd = open(source, O_RDONLY | O_NONBLOCK);
    if (src_fd < 0) {
        printf("Failed to open source file\n");
        exit(EXIT_FAILURE);
    }

    dst_fd = open(destination, O_CREAT | O_WRONLY | O_TRUNC | O_NONBLOCK, 0666);
    if (dst_fd < 0) {
        printf("Failed to open destination file\n");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    if (fstat(src_fd, &st) < 0) {
        printf("Failed to get file size\n");
        close(src_fd);
        close(dst_fd);
        exit(EXIT_FAILURE);
    }

    file_size = st.st_size;
    offset = 0;
    aio_list = (struct aiocb **)malloc(num_operations * sizeof(struct aiocb*));
    if (!aio_list) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    start = clock();
    while (offset < file_size) {
        active_ops = 0;
        for (i = 0; i < num_operations && offset < file_size; i++) {
            struct aio_operation *op = (struct aio_operation *)malloc(sizeof(struct aio_operation));
            if (!op) {
                printf("Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            op->buffer = (char *)malloc(block_size);
            if (!op->buffer) {
                printf("Memory allocation failed\n");
                exit(EXIT_FAILURE);
            }
            op->write_operation = 0;
            setup_aio_operation(op, src_fd, offset, block_size, 0);
            aio_list[i] = &op->aio;

            if (aio_read(&op->aio) < 0) {
                printf("Failed to start aio_read\n");
                exit(EXIT_FAILURE);
            }

            offset += block_size;
            active_ops++;
        }

        for (i = 0; i < active_ops; i++) {
            if (aio_suspend((const struct aiocb *const *)&aio_list[i], 1, NULL) < 0) {
                printf("aio_suspend failed\n");
                exit(EXIT_FAILURE);
            }
            if (aio_error(aio_list[i]) == 0) {
                ssize_t read_bytes = aio_return(aio_list[i]);
                if (read_bytes > 0) {
                    struct aio_operation *write_op = (struct aio_operation *)malloc(sizeof(struct aio_operation));
                    if (!write_op) {
                        printf("Memory allocation failed\n");
                        exit(EXIT_FAILURE);
                    }
                    write_op->buffer = ((struct aio_operation *)aio_list[i]->aio_sigevent.sigev_value.sival_ptr)->buffer;
                    write_op->write_operation = 1;
                    __sync_fetch_and_add(&active_writes, 1);
                    setup_aio_operation(write_op, dst_fd, aio_list[i]->aio_offset, read_bytes, 1);
                    if (aio_write(&write_op->aio) < 0) {
                        printf("Failed to start aio_write\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }

    while (active_writes > 0) {}
    end = clock();
    printf("Copy completed in %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    free(aio_list);
    close(src_fd);
    close(dst_fd);
}

int main(int argc, char *argv[]) {
    int num_operations;
    size_t block_size;
    
    if (argc != 4) {
        printf("Usage: %s <source> <destination> <num_operations>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_operations = atoi(argv[3]);
    if (num_operations <= 0) {
        printf("Invalid number of operations\n");
        exit(EXIT_FAILURE);
    }

    block_size = BUFFER_SIZE;
    copy_file(argv[1], argv[2], block_size, num_operations);
    return 0;
}