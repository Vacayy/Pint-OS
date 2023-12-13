#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"

#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include "devices/input.h"
#include "lib/kernel/stdio.h"

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
	
	lock_init(&filesys_lock);
}

/* System Call functions */
void
halt (void) {
	power_off();
}

void
exit (int status) {
	struct thread *curr = thread_current();
	printf("%s: exit(%d)\n", curr->name, status);
	curr -> exit_status = status;
	thread_exit();
}

bool 
create(const char *file, unsigned initial_size) {
	check_address(file);
	return filesys_create(file, initial_size);
}

bool 
remove(const char *file) {
	check_address(file);
	return filesys_remove(file);
}

int 
open(const char *file) {
	check_address(file);
	struct file *opend_file = filesys_open(file);
	if (opend_file == NULL)
		return -1;

	int fd = process_add_file(opend_file);
	if (fd == -1)
		file_close(opend_file);

	return fd;
}

int 
filesize(int fd) {
	struct file *file = process_get_file(fd);
	if (file == NULL){
		return -1;
	}
	return file_length(file);
}

void 
seek(int fd, unsigned position){
	if (fd < 2){
		return;
	}
	struct file *file = process_get_file(fd);
	if (file == NULL){
		return;
	}
	file_seek(file, position);
}

unsigned 
tell(int fd){
	if (fd < 2){
		return;
	}
	
	struct file *file = process_get_file(fd);

	if (file == NULL){
		return;
	}
	
	return file_tell(file);
}

void 
close(int fd){
	if (fd < 2){
		return;		
	}	

	struct file *file = process_get_file(fd);
	
	if (file == NULL){
		return;
	}

	file_close(file);
	process_close_file(fd);
}

int 
read (int fd, void *buffer, unsigned size) {
	check_address(buffer);

	char *ptr = (char *)buffer;
	int bytes_read = 0;

	if (fd == 0){ // STDIN_FILENO
		for (int i = 0; i < size; i++){
			char ch = input_getc();
			if (ch == '\n'){
				break;
			}
			*ptr = ch;
			ptr ++;
			bytes_read++;
		}
	} else {
		if (fd < 2){
			return -1;
		}
		
		struct file *file = process_get_file(fd);
		
		if (file == NULL){
			return -1;
		}

		lock_acquire(&filesys_lock);
		bytes_read = file_read(file, buffer, size);
		lock_release(&filesys_lock);
	}

	return bytes_read;
}

int 
write (int fd, const void *buffer, unsigned size){
	check_address(buffer);
	int bytes_write = 0;

	if (fd == 1){ // STDOUT_FILENO
		putbuf(buffer, size);
		bytes_write = size;
	} else {
		if (fd < 2){
			return -1;
		}
		struct file *file = process_get_file(fd);
		if (file == NULL){
			return -1;
		}

		lock_acquire(&filesys_lock);
		bytes_write = file_write(file, buffer, size);
		lock_release(&filesys_lock);
	}
	return bytes_write;
}

void 
check_address(void *addr) {
	if (addr == NULL) {
		exit(-1);
	}

	if (!is_user_vaddr (addr)) {
		exit(-1);
	}

	if (pml4_get_page(thread_current()->pml4, addr) == NULL){
		exit(-1);
	}
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {		
	int syscall_n = f->R.rax; /* syscall number */
	switch (syscall_n)
	{	
	case SYS_HALT:
		halt(); 

	case SYS_EXIT:
		exit(f->R.rdi);
	
	case SYS_CREATE:
		create(f->R.rdi, f->R.rsi);

	case SYS_REMOVE:
		remove(f->R.rdi);

	case SYS_OPEN:
		open(f->R.rdi); 
	
	case SYS_FILESIZE:
		filesize(f->R.rdi); 

	case SYS_SEEK:
		seek(f->R.rdi, f->R.rsi);

	case SYS_TELL:
		tell(f->R.rdi);

	case SYS_CLOSE:
		close(f->R.rdi);

	case SYS_READ:
		read(f->R.rdi, f->R.rsi, f->R.rdx);

	case SYS_WRITE:
		write(f->R.rdi, f->R.rsi, f->R.rdx);
	
	// case SYS_EXEC:
	// 	exec(f->R.rdi);		

	// case SYS_WAIT:
	// 	wait(f->R.rdi);		

	// case SYS_FORK:
	// 	fork(f->R.rdi);

	default:
		printf ("system call!\n");
		break;
	}
	thread_exit ();
}
