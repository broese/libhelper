/*! \file
 * Macros for reading and writing byte streams and swapping bytes
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////
/**
 * @name ByteSwappingMacros
 * Byte swapping macros
 */

/*! \brief Byte-swap a 16-bit value
 * \param v Source value
 * \return Byte-swapped value
 */
static inline uint16_t lh_bswap_short(uint16_t v) {
#ifdef HAVE_BUILTIN_BSWAP16
    return __builtin_bswap16(v);
#else
    return (v>>8)|(v<<8);
#endif
}

/*! \brief Byte-swap a 32-bit value
 * \param v Source value
 * \return Byte-swapped value
 */
static inline uint32_t lh_bswap_int(uint32_t v) {
#ifdef HAVE_BUILTIN_BSWAP32
    return __builtin_bswap32(v);
#else
    return (v<<24) | (v<<8 & 0xff0000) | (v>>8 & 0xff00) | (v>>24);
#endif
}

/*! \brief Byte-swap a 64-bit value
 * \param v Source value
 * \return Byte-swapped value
 */
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

/*! \brief Byte-swap a 32-bit float value
 * \param v Source value
 * \return Byte-swapped value
 */
static inline float lh_bswap_float(float v) {
    union {
        float vf;
        uint32_t vi;
    } S;
    S.vf = v;
    S.vi = lh_bswap_int(S.vi);
    return S.vf;
}

/*! \brief Byte-swap a 64-bit double value
 * \param v Source value
 * \return Byte-swapped value
 */
static inline double lh_bswap_double(double v) {
    union {
        double vd;
        uint64_t vl;
    } S;
    S.vd = v;
    S.vl = lh_bswap_long(S.vl);
    return S.vd;
}

/**
 * @name InplaceByteSwapMacros
 * Perform byteswap in-place
 */
#define lh_ibswap_short(v)  v=lh_bswap_short(v)
#define lh_ibswap_int(v)    v=lh_bswap_int(v)
#define lh_ibswap_long(v)   v=lh_bswap_long(v)
#define lh_ibswap_float(v)  v=lh_bswap_float(v)
#define lh_ibswap_double(v) v=lh_bswap_double(v)

////////////////////////////////////////////////////////////////////////////////

/**
 * @name ParseBytestream
 * Functions for parsing values from a bytestream
 */

#define GETF *t++ = *p++
#define GETR *t-- = *p++

#define GETF1 GETF
#define GETR1 GETR
#define GETF2 GETF;GETF
#define GETR2 GETR;GETR
#define GETF4 GETF2;GETF2
#define GETR4 GETR2;GETR2
#define GETF8 GETF4;GETF4
#define GETR8 GETR4;GETR4
#define GETFM(n) GETF ## n
#define GETRM(n) GETR ## n

// Note: we are using a "statement expression" ( { } ) to turn
// a block into an expression. This is a GCC-specific extension,
// not standard C
//FIXME: introduce a C-standard variant of this macro, distinguish with #ifdef __GNU_C

#if __BYTE_ORDER == __LITTLE_ENDIAN     

#define lh_parse_be(ptr,type,size) ( {          \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[size-1];       \
        GETRM(size);                            \
        temp.value; } )
#define lh_parse_le(ptr,type,size) ( {          \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[0];            \
        GETFM(size);                            \
        temp.value; } )

#else

#define lh_parse_be(ptr,type,size) ( {          \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[0];            \
        GETFM(size);                            \
        temp.value; } )
#define lh_parse_le(ptr,type,size) ( {          \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[size-1];       \
        GETRM(size);                            \
        temp.value; } )

#endif                                  

#define lh_parse_char_be(ptr)   *ptr
#define lh_parse_char_le(ptr)   *ptr
#define lh_parse_short_be(ptr)  lh_parse_be(ptr,uint16_t,2)
#define lh_parse_short_le(ptr)  lh_parse_le(ptr,uint16_t,2)
#define lh_parse_int_be(ptr)    lh_parse_be(ptr,uint32_t,4)
#define lh_parse_int_le(ptr)    lh_parse_le(ptr,uint32_t,4)
#define lh_parse_long_be(ptr)   lh_parse_be(ptr,uint64_t,8)
#define lh_parse_long_le(ptr)   lh_parse_le(ptr,uint64_t,8)
#define lh_parse_float_be(ptr)  lh_parse_be(ptr,float,4)
#define lh_parse_float_le(ptr)  lh_parse_le(ptr,float,4)
#define lh_parse_double_be(ptr) lh_parse_be(ptr,double,8)
#define lh_parse_double_le(ptr) lh_parse_le(ptr,double,8)

////////////////////////////////////////////////////////////////////////////////

/**
 * @name ReadByteStream
 * Macros that parse the byte stream for data and then advance the reading pointer
 */
#define lh_read_be(ptr,type,size)                                       \
    ( { type temp = lh_parse_be(ptr,type,size); ptr+=size; temp; } )
#define lh_read_le(ptr,type,size)                                       \
    ( { type temp = lh_parse_le(ptr,type,size); ptr+=size; temp; } )

#define lh_read_char_be(ptr)    *ptr++
#define lh_read_char_le(ptr)    lh_read_char_be(ptr)
#define lh_read_short_be(ptr)   lh_read_be(ptr,uint16_t,2)
#define lh_read_short_le(ptr)   lh_read_le(ptr,uint16_t,2)
#define lh_read_int_be(ptr)     lh_read_be(ptr,uint32_t,4)
#define lh_read_int_le(ptr)     lh_read_le(ptr,uint32_t,4)
#define lh_read_long_be(ptr)    lh_read_be(ptr,uint64_t,8)
#define lh_read_long_le(ptr)    lh_read_le(ptr,uint64_t,8)
#define lh_read_float_be(ptr)   lh_read_be(ptr,float,4)
#define lh_read_float_le(ptr)   lh_read_le(ptr,float,4)
#define lh_read_double_be(ptr)  lh_read_be(ptr,double,8)
#define lh_read_double_le(ptr)  lh_read_le(ptr,double,8)

////////////////////////////////////////////////////////////////////////////////

/**
 * @name ReadLimitByteStream
 * Macros for reading the byte stream with limit checking
 */
#define lh_lread_be(ptr,lim,type,var,size,fail) \
    if (lim-ptr < size) { fail; }               \
    type var = lh_read_be(ptr,type,size);
#define lh_lread_le(ptr,lim,type,var,size,fail) \
    if (lim-ptr < size) { fail; }               \
    type var = lh_read_le(ptr,type,size);

#define lh_lread_char_be(ptr,lim,var,fail)      \
    if (lim==ptr) { fail; }                     \
    uint8_t var = *ptr++;
#define lh_lread_char_le(ptr,lim,var,fail)    lh_lread_char_be(ptr,lim,var,fail)

#define lh_lread_short_be(ptr,lim,var,fail)   lh_lread_be(ptr,lim,uint16_t,var,2,fail)
#define lh_lread_short_le(ptr,lim,var,fail)   lh_lread_le(ptr,lim,uint16_t,var,2,fail)
#define lh_lread_int_be(ptr,lim,var,fail)     lh_lread_be(ptr,lim,uint32_t,var,4,fail)
#define lh_lread_int_le(ptr,lim,var,fail)     lh_lread_le(ptr,lim,uint32_t,var,4,fail)
#define lh_lread_long_be(ptr,lim,var,fail)    lh_lread_be(ptr,lim,uint64_t,var,8,fail)
#define lh_lread_long_le(ptr,lim,var,fail)    lh_lread_le(ptr,lim,uint64_t,var,8,fail)
#define lh_lread_float_be(ptr,lim,var,fail)   lh_lread_be(ptr,lim,float,var,4,fail)
#define lh_lread_float_le(ptr,lim,var,fail)   lh_lread_le(ptr,lim,float,var,4,fail)
#define lh_lread_double_be(ptr,lim,var,fail)  lh_lread_be(ptr,lim,double,var,8,fail)
#define lh_lread_double_le(ptr,lim,var,fail)  lh_lread_le(ptr,lim,double,var,8,fail)






#if 0


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
