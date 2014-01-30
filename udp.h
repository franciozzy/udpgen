#define UDPPORT 41300

typedef struct {
    unsigned int       seq;
    unsigned long long tsctx;
    unsigned long long tscrx;
}__attribute__((packed)) udpdata_t;

unsigned long long rdtsc();

unsigned long long get_hz();
