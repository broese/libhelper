/*! \file
 * Macros for reading and writing byte streams and swapping bytes
 */

#pragma once

////////////////////////////////////////////////////////////////////////////////
/**
 * @name Byte Swapping Macros
 * Macros for swapping byteorder in elemental types.
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
 * @name Parse Bytestream
 * Functions for parsing values from a bytestream
 */

#define GETBE v = (v<<8)|(*((*p)++));

static inline uint8_t _lh_read_char(uint8_t **p) {
    uint8_t v = *((*p)++);
    return v;
}

static inline uint16_t _lh_read_short_be(uint8_t **p) {
    uint16_t v=(uint16_t)*((*p)++);
    GETBE;
    return v;
}

static inline uint16_t _lh_read_short_le(uint8_t **p) {
    uint16_t v=(uint16_t)*((*p)++);
    v |= ((*((*p)++))<<8);
    return v;
}

static inline uint32_t _lh_read_int_be(uint8_t **p) {
    uint32_t v=(uint32_t)*((*p)++);
    GETBE;GETBE;GETBE;
    return v;
}

static inline uint32_t _lh_read_int_le(uint8_t **p) {
    uint32_t v=(uint32_t)*((*p)++);
    v |= ((*((*p)++))<<8); v |= ((*((*p)++))<<16); v |= ((*((*p)++))<<24);
    return v;
}

static inline uint64_t _lh_read_long_be(uint8_t **p) {
    uint64_t v=(uint64_t)*((*p)++);
    GETBE;GETBE;GETBE;GETBE;GETBE;GETBE;GETBE;
    return v;
}

static inline uint64_t _lh_read_long_le(uint8_t **p) {
    uint64_t v=(uint64_t)*((*p)++);
    v |= ((*((*p)++))<<8); v |= ((*((*p)++))<<16); v |= ((*((*p)++))<<24);
    v |= ((uint64_t)(*((*p)++))<<32); v |= ((uint64_t)(*((*p)++))<<40);
    v |= ((uint64_t)(*((*p)++))<<48); v |= ((uint64_t)(*((*p)++))<<56);
    return v;
}

static inline float _lh_read_float_be(uint8_t **p) {
    union { uint32_t i; float f; } temp;
    temp.i = _lh_read_int_be(p);
    return temp.f;
}

static inline float _lh_read_float_le(uint8_t **p) {
    union { uint32_t i; float f; } temp;
    temp.i = _lh_read_int_le(p);
    return temp.f;
}

static inline double _lh_read_double_be(uint8_t **p) {
    union { uint64_t l; double d; } temp;
    temp.l = _lh_read_long_be(p);
    return temp.d;
}

static inline double _lh_read_double_le(uint8_t **p) {
    union { uint64_t l; double d; } temp;
    temp.l = _lh_read_long_le(p);
    return temp.d;
}

#define lh_read_char(p)         _lh_read_char(&p)
#define lh_read_short_be(p)     _lh_read_short_be(&p)
#define lh_read_short_le(p)     _lh_read_short_le(&p)
#define lh_read_int_be(p)       _lh_read_int_be(&p)
#define lh_read_int_le(p)       _lh_read_int_le(&p)
#define lh_read_long_be(p)      _lh_read_long_be(&p)
#define lh_read_long_le(p)      _lh_read_long_le(&p)
#define lh_read_float_be(p)     _lh_read_float_be(&p)
#define lh_read_float_le(p)     _lh_read_float_le(&p)
#define lh_read_double_be(p)    _lh_read_double_be(&p)
#define lh_read_double_le(p)    _lh_read_double_le(&p)

static inline uint8_t  lh_parse_char(uint8_t *p)      { _lh_read_char(&p); }
static inline uint16_t lh_parse_short_be(uint8_t *p)  { _lh_read_short_be(&p); }
static inline uint16_t lh_parse_short_le(uint8_t *p)  { _lh_read_short_le(&p); }
static inline uint32_t lh_parse_int_be(uint8_t *p)    { _lh_read_int_be(&p); }
static inline uint32_t lh_parse_int_le(uint8_t *p)    { _lh_read_int_le(&p); }
static inline uint64_t lh_parse_long_be(uint8_t *p)   { _lh_read_long_be(&p); }
static inline uint64_t lh_parse_long_le(uint8_t *p)   { _lh_read_long_le(&p); }
static inline float    lh_parse_float_be(uint8_t *p)  { _lh_read_float_be(&p); }
static inline float    lh_parse_float_le(uint8_t *p)  { _lh_read_float_le(&p); }
static inline double   lh_parse_double_be(uint8_t *p) { _lh_read_double_be(&p); }
static inline double   lh_parse_double_le(uint8_t *p) { _lh_read_double_le(&p); }

//TODO: zstring, lstring, varint

#define def_lread(name,type)                                            \
    static inline int _lh_lread_##name(uint8_t **p, uint8_t *l, type *v) { \
        if (l<*p+sizeof(*v)) return 0;                                  \
        *v = _lh_read_##name(p);                                        \
        return 1;                                                       \
    }

def_lread(char,uint8_t);
def_lread(short_be,uint16_t);
def_lread(short_le,uint16_t);
def_lread(int_be,uint32_t);
def_lread(int_le,uint32_t);
def_lread(long_be,uint64_t);
def_lread(long_le,uint64_t);
def_lread(float_be,float);
def_lread(float_le,float);
def_lread(double_be,double);
def_lread(double_le,double);

#define lh_lread_char(p,l,v)        _lh_lread_char(&p,l,&v)
#define lh_lread_char_be(p,l,v)     _lh_lread_char_be(&p,l,&v)
#define lh_lread_char_le(p,l,v)     _lh_lread_char_le(&p,l,&v)
#define lh_lread_short_be(p,l,v)    _lh_lread_short_be(&p,l,&v)
#define lh_lread_short_le(p,l,v)    _lh_lread_short_le(&p,l,&v)
#define lh_lread_int_be(p,l,v)      _lh_lread_int_be(&p,l,&v)
#define lh_lread_int_le(p,l,v)      _lh_lread_int_le(&p,l,&v)
#define lh_lread_long_be(p,l,v)     _lh_lread_long_be(&p,l,&v)
#define lh_lread_long_le(p,l,v)     _lh_lread_long_le(&p,l,&v)
#define lh_lread_float_be(p,l,v)    _lh_lread_float_be(&p,l,&v)
#define lh_lread_float_le(p,l,v)    _lh_lread_float_le(&p,l,&v)
#define lh_lread_double_be(p,l,v)   _lh_lread_double_be(&p,l,&v)
#define lh_lread_double_le(p,l,v)   _lh_lread_double_le(&p,l,&v)

////////////////////////////////////////////////////////////////////////////////

#if 0

#define lh_parse_varint(ptr) ( {                \
            uint32_t temp = 0;                  \
            uint8_t *p = ptr;                   \
            int s = 0;                          \
            uint8_t c;                          \
            do {                                \
                c=*p++;                         \
                temp += ((c&0x7f)<<s);          \
                s += 7;                         \
            } while (c&0x80);                   \
            temp; })

#define lh_read_varint(ptr) ( {                 \
            uint32_t temp = 0;                  \
            int s = 0;                          \
            uint8_t c;                          \
            do {                                \
                c=*ptr++;                       \
                temp += ((c&0x7f)<<s);          \
                s += 7;                         \
            } while (c&0x80);                   \
            temp; })
#endif

////////////////////////////////////////////////////////////////////////////////

#if 0
/**
 * @name Read Limit Bytestream
 * Macros for reading the byte stream with limit checking
 */

/// \cond
#define lh_lread_be(ptr,lim,type,var,size,fail) \
    if (lim-ptr < size) { fail; }               \
    type var = lh_read_be(ptr,type,size);
#define lh_lread_le(ptr,lim,type,var,size,fail) \
    if (lim-ptr < size) { fail; }               \
    type var = lh_read_le(ptr,type,size);
/// \endcond

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

#endif
////////////////////////////////////////////////////////////////////////////////

/**
 * @name Place to Byte Stream
 * Write elemental data types to a byte stream
 */

/// \cond INTERNAL
#define PUTF *p++ = *t++
#define PUTR *p++ = *t--

#define PUTF1 PUTF
#define PUTR1 PUTR
#define PUTF2 PUTF;PUTF
#define PUTR2 PUTR;PUTR
#define PUTF4 PUTF2;PUTF2
#define PUTR4 PUTR2;PUTR2
#define PUTF8 PUTF4;PUTF4
#define PUTR8 PUTR4;PUTR4
#define PUTFM(n) PUTF ## n
#define PUTRM(n) PUTR ## n

// Note: we are using a "statement expression" ( { } ) to turn
// a block into an expression. This is a GCC-specific extension,
// not standard C
//FIXME: introduce a C-standard variant of this macro,
//distinguish with #ifdef __GNU_C

#if __BYTE_ORDER == __LITTLE_ENDIAN     

#define lh_place_be(ptr,type,size,val) ( {      \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        temp.value = val;                       \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[size-1];       \
        PUTRM(size);                            \
        p; } )
#define lh_place_le(ptr,type,size,val) ( {      \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        temp.value = val;                       \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[0];            \
        PUTFM(size);                            \
        p; } )

#else

#define lh_place_be(ptr,type,size,val) ( {      \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        temp.value = val;                       \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[0];            \
        PUTFM(size);                            \
        p; } )
#define lh_place_le(ptr,type,size,val) ( {      \
        union {                                 \
            uint8_t bytes[size];                \
            type    value;                      \
        } temp;                                 \
        temp.value = val;                       \
        uint8_t *p = (uint8_t *)ptr;            \
        uint8_t *t = &temp.bytes[size-1];       \
        PUTRM(size);                            \
        p; } )

#endif           

/// \endcond

#define lh_place_char_be(ptr,val)               \
    ( { uint8_t *p = (uint8_t *)ptr; *p++=val; p; } )
#define lh_place_char_le(ptr,val)       lh_place_char_be(ptr,val)
#define lh_place_short_be(ptr,val)      lh_place_be(ptr,uint16_t,2,val)
#define lh_place_short_le(ptr,val)      lh_place_le(ptr,uint16_t,2,val)
#define lh_place_int_be(ptr,val)        lh_place_be(ptr,uint32_t,4,val)
#define lh_place_int_le(ptr,val)        lh_place_le(ptr,uint32_t,4,val)
#define lh_place_long_be(ptr,val)       lh_place_be(ptr,uint64_t,8,val)
#define lh_place_long_le(ptr,val)       lh_place_le(ptr,uint64_t,8,val)
#define lh_place_float_be(ptr,val)      lh_place_be(ptr,float,4,val)
#define lh_place_float_le(ptr,val)      lh_place_le(ptr,float,4,val)
#define lh_place_double_be(ptr,val)     lh_place_be(ptr,double,8,val)
#define lh_place_double_le(ptr,val)     lh_place_le(ptr,double,8,val)

#define lh_place_varint(ptr,val) ( {             \
            uint32_t temp = (uint32_t) val;      \
            uint8_t *p = ptr;                    \
            while(temp>0) {                      \
                *p++ = (temp&0x7f)|0x80;         \
                temp >>= 7;                      \
            }                                    \
            *(p-1) &= 0x7f;                      \
            p; } )
                


/**
 * @name Write Byte Stream
 * Write elemental types to a bytestream and advance the write pointer
 */

#define lh_write_char_be(ptr,val)       ptr=lh_place_char_be(ptr,val)
#define lh_write_char_le(ptr,val)       ptr=lh_place_char_le(ptr,val)
#define lh_write_short_be(ptr,val)      ptr=lh_place_short_be(ptr,val)
#define lh_write_short_le(ptr,val)      ptr=lh_place_short_le(ptr,val)
#define lh_write_int_be(ptr,val)        ptr=lh_place_int_be(ptr,val)
#define lh_write_int_le(ptr,val)        ptr=lh_place_int_le(ptr,val)
#define lh_write_long_be(ptr,val)       ptr=lh_place_long_be(ptr,val)
#define lh_write_long_le(ptr,val)       ptr=lh_place_long_le(ptr,val)
#define lh_write_float_be(ptr,val)      ptr=lh_place_float_be(ptr,val)
#define lh_write_float_le(ptr,val)      ptr=lh_place_float_le(ptr,val)
#define lh_write_double_be(ptr,val)     ptr=lh_place_double_be(ptr,val)
#define lh_write_double_le(ptr,val)     ptr=lh_place_double_le(ptr,val)

#define lh_write_varint(ptr,val)        ptr=lh_place_varint(ptr,val)

////////////////////////////////////////////////////////////////////////////////

#if 0
#include <stdarg.h>

#define UNPACK(name,type)                        \
    { lh_lread_##name(ptr,lim,var,fail++;break); \
        *(va_arg(args,type *)) = var; break; }

static inline ssize_t lh_unpack(uint8_t *ptr, uint8_t *lim, const char *fmt, ...) {
    va_list args;
    va_start(args,fmt);

    int fail = 0;
    uint8_t * start = ptr;

    for(;fmt[0];fmt++) {
        switch(fmt[0]) {
        case 'c': 
        case 'C': UNPACK(char_be,uint8_t);
        case 's': UNPACK(short_le,uint16_t);
        case 'S': UNPACK(short_be,uint16_t);
        case 'i': UNPACK(int_le,uint32_t);
        case 'I': UNPACK(int_be,uint32_t);
        case 'l': UNPACK(long_le,uint64_t);
        case 'L': UNPACK(long_be,uint64_t);
        case 'f': UNPACK(float_le,float);
        case 'F': UNPACK(float_be,float);
        case 'd': UNPACK(double_le,double);
        case 'D': UNPACK(double_be,double);
        default: fail = 1; break;
        }
        if (fail) break;
    }

    va_end(args);

    if (fail)
        return -1;
    else
        return ptr-start;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define bswap_short(v)                  lh_bswap_short(v)
#define bswap_int(v)                    lh_bswap_int(v)
#define bswap_long(v)                   lh_bswap_long(v)
#define bswap_float(v)                  lh_bswap_float(v)
#define bswap_double(v)                 lh_bswap_double(v)

#define ibswap_short(v)                 lh_ibswap_short(v)
#define ibswap_int(v)                   lh_ibswap_int(v)
#define ibswap_long(v)                  lh_ibswap_long(v)
#define ibswap_float(v)                 lh_ibswap_float(v)
#define ibswap_double(v)                lh_ibswap_double(v)

#define parse_char(ptr)                 lh_parse_char_be(ptr)
#define parse_char_le(ptr)              lh_parse_char_le(ptr)
#define parse_short(ptr)                lh_parse_short_be(ptr)
#define parse_short_le(ptr)             lh_parse_short_le(ptr)
#define parse_int(ptr)                  lh_parse_int_be(ptr)
#define parse_int_le(ptr)               lh_parse_int_le(ptr)
#define parse_long(ptr)                 lh_parse_long_be(ptr)
#define parse_long_le(ptr)              lh_parse_long_le(ptr)
#define parse_float(ptr)                lh_parse_float_be(ptr)
#define parse_float_le(ptr)             lh_parse_float_le(ptr)
#define parse_double(ptr)               lh_parse_double_be(ptr)
#define parse_double_le(ptr)            lh_parse_double_le(ptr)

#define lread_char(ptr,lim,var,fail)        lh_lread_char_be(ptr,lim,var,fail)
#define lread_char_le(ptr,lim,var,fail)     lh_lread_char_le(ptr,lim,var,fail)
#define lread_short(ptr,lim,var,fail)       lh_lread_short_be(ptr,lim,var,fail)
#define lread_short_le(ptr,lim,var,fail)    lh_lread_short_le(ptr,lim,var,fail)
#define lread_int(ptr,lim,var,fail)         lh_lread_int_be(ptr,lim,var,fail)
#define lread_int_le(ptr,lim,var,fail)      lh_lread_int_le(ptr,lim,var,fail)
#define lread_long(ptr,lim,var,fail)        lh_lread_long_be(ptr,lim,var,fail)
#define lread_long_le(ptr,lim,var,fail)     lh_lread_long_le(ptr,lim,var,fail)
#define lread_float(ptr,lim,var,fail)       lh_lread_float_be(ptr,lim,var,fail)
#define lread_float_le(ptr,lim,var,fail)    lh_lread_float_le(ptr,lim,var,fail)
#define lread_double(ptr,lim,var,fail)      lh_lread_double_be(ptr,lim,var,fail)
#define lread_double_le(ptr,lim,var,fail)   lh_lread_double_le(ptr,lim,var,fail)

#define place_char(ptr,val)             lh_place_char_be(ptr,val)
#define place_char_le(ptr,val)          lh_place_char_le(ptr,val)
#define place_short(ptr,val)            lh_place_short_be(ptr,val)
#define place_short_le(ptr,val)         lh_place_short_le(ptr,val)
#define place_int(ptr,val)              lh_place_int_be(ptr,val)
#define place_int_le(ptr,val)           lh_place_int_le(ptr,val)
#define place_long(ptr,val)             lh_place_long_be(ptr,val)
#define place_long_le(ptr,val)          lh_place_long_le(ptr,val)
#define place_float(ptr,val)            lh_place_float_be(ptr,val)
#define place_float_le(ptr,val)         lh_place_float_le(ptr,val)
#define place_double(ptr,val)           lh_place_double_be(ptr,val)
#define place_double_le(ptr,val)        lh_place_double_le(ptr,val)

#define write_char(ptr,val)             lh_write_char_be(ptr,val)
#define write_char_le(ptr,val)          lh_write_char_le(ptr,val)
#define write_short(ptr,val)            lh_write_short_be(ptr,val)
#define write_short_le(ptr,val)         lh_write_short_le(ptr,val)
#define write_int(ptr,val)              lh_write_int_be(ptr,val)
#define write_int_le(ptr,val)           lh_write_int_le(ptr,val)
#define write_long(ptr,val)             lh_write_long_be(ptr,val)
#define write_long_le(ptr,val)          lh_write_long_le(ptr,val)
#define write_float(ptr,val)            lh_write_float_be(ptr,val)
#define write_float_le(ptr,val)         lh_write_float_le(ptr,val)
#define write_double(ptr,val)           lh_write_double_be(ptr,val)
#define write_double_le(ptr,val)        lh_write_double_le(ptr,val)
#define write_varint                    lh_write_varint

#endif
