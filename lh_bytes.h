/*! \file
 * Macros for reading and writing byte streams and swapping bytes
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////
/**
 * @name ByteSwappingMacros
 * Byte swapping macros
 */

static inline uint16_t lh_bswap_short(uint16_t v) {
#ifdef HAVE_BUILTIN_BSWAP16
    return __builtin_bswap16(v);
#else
    return (v>>8)|(v<<8);
#endif
}

static inline uint32_t lh_bswap_int(uint32_t v) {
#ifdef HAVE_BUILTIN_BSWAP32
    return __builtin_bswap32(v);
#else
    return (v<<24) | (v<<8 & 0xff0000) | (v>>8 & 0xff00) | (v>>24);
#endif
}

static inline uint64_t lh_bswap_long(uint64_t v) {
#ifdef HAVE_BUILTIN_BSWAP64
    return __builtin_bswap64(v);
#else
    return
        (v << 56) |                            \
        ((v & 0x000000000000ff00ULL) << 40) |  \
        ((v & 0x0000000000ff0000ULL) << 24) |  \
        ((v & 0x00000000ff000000ULL) <<  8) |  \
        ((v & 0x000000ff00000000ULL) >>  8) |  \
        ((v & 0x0000ff0000000000ULL) >> 24) |  \
        ((v & 0x00ff000000000000ULL) >> 40) |  \
        (v >> 56);
#endif
}

static inline float lh_bswap_float(float v) {
    union {
        float vf;
        uint32_t vi;
    } S;
    S.vf = v;
    S.vi = lh_bswap_int(S.vi);
    return S.vf;
}

static inline double lh_bswap_double(double v) {
    union {
        double vd;
        uint64_t vl;
    } S;
    S.vd = v;
    S.vl = lh_bswap_long(S.vl);
    return S.vd;
}

#if 0

////////////////////////////////////////////////////////////////////////////////
// Extraction of values from a byte stream

#define GETF *t++ = *p++
#define GETR *t-- = *p++

static inline int8_t parse_char(const char *p) {
    return *p;
}

// Big-Endian extraction (aka network order)

static inline int16_t parse_short(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[1];
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
#endif
    return temp.val;
}

static inline int32_t parse_int(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

static inline int64_t parse_long(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

#define read_char(p)  parse_char(p);  (p)++
#define read_short(p) parse_short(p); (p)+=2
#define read_int(p)   parse_int(p);   (p)+=4
#define read_long(p)  parse_long(p);  (p)+=8

// Little-Endian extraction (aka Intel order)

static inline int16_t parse_short_le(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[1];
    GETR; GETR;
#endif
    return temp.val;
}

static inline int32_t parse_int_le(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

static inline int64_t parse_long_le(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

#define read_char_le(p)  parse_char(p);     (p)++
#define read_short_le(p) parse_short_le(p); (p)+=2
#define read_int_le(p)   parse_int_le(p);   (p)+=4
#define read_long_le(p)  parse_long_le(p);  (p)+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

static inline double parse_double(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

#define read_float(p)   parse_float(p);   p+=4
#define read_double(p)  parse_double(p);  p+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float_le(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

static inline double parse_double_le(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

#define read_float_le(p)   parse_float_le(p);   p+=4
#define read_double_le(p)  parse_double_le(p);  p+=8

////////////////////////////////////////////////////////////////////////////////
// Writing to a stream

#define PUTF *p++ = *t++
#define PUTR *p++ = *t--

static inline char * place_char(char *p, int8_t v) {
    *p++ = v;
    return p;
}

static inline char * place_short(char *p, int16_t v) {
    union {
        int8_t  buf[2];
        int16_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[1];
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
#endif

    return p;
}

static inline char * place_int(char *p, int32_t v) {
    union {
        int8_t  buf[4];
        int32_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif

    return p;
}

static inline char * place_long(char *p, int64_t v) {
    union {
        int8_t  buf[8];
        int64_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif

    return p;
}

#define write_char(p,v)  p=place_char(p,v)
#define write_short(p,v) p=place_short(p,v)
#define write_int(p,v)   p=place_int(p,v)
#define write_long(p,v)  p=place_long(p,v)


static inline uint8_t * place_float(char *p, float v) {
    union {
        uint8_t buf[4];
        float val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    uint8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return p;
}

static inline uint8_t * place_double(char *p, double v) {
    union {
        uint8_t buf[8];
        double val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    uint8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return p;
}

#define write_float(p,v)   place_float(p,v);   p+=4
#define write_double(p,v)  place_double(p,v);  p+=8

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define bswap_short(v)                  lh_bswap_short(v)
#define bswap_int(v)                    lh_bswap_int(v)
#define bswap_long(v)                   lh_bswap_long(v)
#define bswap_float(v)                  lh_bswap_float(v)
#define bswap_double(v)                 lh_bswap_double(v)

#endif
