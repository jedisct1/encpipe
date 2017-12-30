#ifndef common_H
#define common_H 1

#define LOAD32_LE(SRC) load32_le(SRC)
static inline uint32_t
load32_le(const uint8_t src[4])
{
    uint32_t w = (uint32_t) src[0];
    w |= (uint32_t) src[1] << 8;
    w |= (uint32_t) src[2] << 16;
    w |= (uint32_t) src[3] << 24;
    return w;
}

#define STORE32_LE(DST, W) store32_le((DST), (W))
static inline void
store32_le(uint8_t dst[4], uint32_t w)
{
    dst[0] = (uint8_t) w;
    w >>= 8;
    dst[1] = (uint8_t) w;
    w >>= 8;
    dst[2] = (uint8_t) w;
    w >>= 8;
    dst[3] = (uint8_t) w;
}

static void
die(int print_errno, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    if (print_errno) {
        fprintf(stderr, "- %s", strerror(errno));
    }
    fprintf(stderr, "\n");

    exit(1);
}

#endif
