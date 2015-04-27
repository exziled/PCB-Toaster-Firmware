#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdint.h>

/**
 * @brief Allocate memory from the heap.
 *
 * The current heap implementation is very rudimentary, it is only able to allocate
 * memory. It does not have any means to free memory again.
 *
 * @return      a pointer to the successfully allocated memory
 * @return      -1 on error, and errno is set to ENOMEM
 */
caddr_t _sbrk ( int incr )
{
extern char _end; /* Defined by the linker */
static char *heap_end;
char *prev_heap_end;
 
  if (heap_end == 0) {
    heap_end = &_end;
  }
 
  prev_heap_end = heap_end;
  //if (prev_heap_end+delta > get_stack_pointer()) {
  //       return (void *) -1L;
  //}
  heap_end += incr;
  return (void *) prev_heap_end;
}