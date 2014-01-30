#include <stdio.h>
#include <string.h>

#if defined(__i386__)
__inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
#elif defined(__x86_64__)
__inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif

unsigned long long get_hz(void){
    FILE               *fp;
    char                buf[1024];
    unsigned long long  ret = 0;
    float               f = 0;

    if ((fp = fopen("/proc/cpuinfo", "r")) == NULL){
        perror("fopen");
        goto err;
    }

    while(fgets(buf, sizeof(buf), fp)){
        if (strncmp(buf, "cpu MHz", 7)){
            continue;
        }
        sscanf(buf, "cpu MHz%*[ \t]: %f\n", &f);
        break;
    }

    ret = (unsigned long long)(f*1000000.0);

out:
    if (fp)
        fclose(fp);

    return(ret);

err:
    ret = 0;
    goto out;
}
