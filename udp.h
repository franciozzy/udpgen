#define UDPPORT 41300

typedef struct {
    unsigned int       seq;
    unsigned long long tsc;
}__attribute__((packed)) udpdata_t;

unsigned long long rdtsc();
