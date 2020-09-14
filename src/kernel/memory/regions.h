#ifndef MISTIX_KERNEL_MEMORY_REGIONS_H
#define MISTIX_KERNEL_MEMORY_REGIONS_H

#define KERNEL_STACK_SIZE 16384

void* get_kernel_start();
void* get_kernel_end();
void* get_kernel_stack_start();
void* get_kernel_stack_end();

#endif // MISTIX_KERNEL_MEMORY_REGIONS_H