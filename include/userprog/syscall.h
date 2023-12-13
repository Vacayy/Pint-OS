#include <stdbool.h>
#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt (void);
void exit (int status);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open (const char *file);
int filesize(int fd);
void seek(int fd, unsigned position);
unsigned tell(int fd);

void check_address (void *addr);

#endif /* userprog/syscall.h */
