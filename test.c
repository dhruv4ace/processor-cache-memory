#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <emmintrin.h>
#include <inttypes.h>

static inline uint64_t rdtscp(void)
{
    uint32_t eax, edx;
    __asm__ __volatile__(
                "rdtscp"
                : "+a" (eax), "=d" (edx)
                :: "%ecx", "memory");


    return (((uint64_t)edx << 32) | eax);
}

int main(int argx, char* argv[]) {

    unsigned char* buffer;
    printf("[+] Allocate Memory for 256 pages\n");
    buffer = mmap(NULL, 256 * 4096, PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    memset(buffer, 0, 256 * 4096);

    printf("[+] Flush the 256 pages out of the cache\n");
    for (size_t k = 0; k<256; k++) {
        _mm_clflush(buffer + k * 4096);
    }
    _mm_mfence();

    buffer[42 * 4096] = 0xff;

    size_t lowest_k = 0;
    uint64_t lowest_dt = 0xffffffff;
    for (size_t k = 0; k < 256; k++) {
        uint64_t t0, dt;

        t0 = rdtscp();
        *(volatile char *)(buffer + 4096 * k);
        dt = rdtscp() - t0;
        if(dt<lowest_dt) {
            lowest_dt = dt;
            lowest_k = k;
        }
        printf("accessing page %02d in %" PRIu64 " cycles \n", k, dt);
    }
}
