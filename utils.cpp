


// this implementation came from
// the Linux rand() man page
// available here: http://linux.die.net/man/3/rand
// This is a POSIX sample implementation.
// I don't need crypto grade random numbers anyway.
 
/* RAND_MAX assumed to be 32767 */

static unsigned long next = 1;

int my_rand(void) {
    next = next * 1103515245 + 12345;
    return((unsigned)(next/65536) % 32768);
}
void my_srand(unsigned seed) {
    next = seed;
}
