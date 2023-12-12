#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void halt (void);
int open (const char *file);

#endif /* userprog/syscall.h */
