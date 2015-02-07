/*! \file
 * Macros for reading and writing byte streams and swapping bytes
 */

#pragma once

#include <stdint.h>

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

static inline uint32_t _lh_read_varint(uint8_t **p) {
    uint32_t v=0;
    int s=0;
    uint8_t c;
    do {
        c = *(*p)++;
        v += ((c&0x7f)<<s);
        s += 7;
    } while (c&0x80);
    return v;
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
#define lh_read_varint(p)       _lh_read_varint(&p)

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
static inline uint32_t lh_parse_varint(uint8_t *p)    { _lh_read_varint(&p); }

//TODO: zstring, lstring

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

static inline int _lh_lread_varint(uint8_t **p, uint8_t *l, uint32_t *v) {
    *v=0;
    int s=0;
    uint8_t c;
    uint8_t *temp = *p;
    do {
        if (*p>=l) {
            *p = temp;
            return 0;
        }
        c = *(*p)++;
        *v += ((c&0x7f)<<s);
        s += 7;
    } while (c&0x80);
    return 1;
}

#define lh_lread_char(p,l,v)        _lh_lread_char(&p,l,&v)
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
#define lh_lread_varint(p,l,v)      _lh_lread_varint(&p,l,&v)

////////////////////////////////////////////////////////////////////////////////

#define PUTLE *p++=(uint8_t)v; v>>=8;

static inline uint8_t * lh_place_char(uint8_t *p, uint8_t v) {
    PUTLE;
    return p;
}

static inline uint8_t * lh_place_short_le(uint8_t *p, uint16_t v) {
    PUTLE;PUTLE;
    return p;
}

static inline uint8_t * lh_place_int_le(uint8_t *p, uint32_t v) {
    PUTLE;PUTLE;PUTLE;PUTLE;
    return p;
}

static inline uint8_t * lh_place_long_le(uint8_t *p, uint64_t v) {
    PUTLE;PUTLE;PUTLE;PUTLE;PUTLE;PUTLE;PUTLE;PUTLE;
    return p;
}

static inline uint8_t * lh_place_short_be(uint8_t *p, uint16_t v) {
    *p++ = (uint8_t)(v>>8);  *p++ = (uint8_t)v;
    return p;
}

static inline uint8_t * lh_place_int_be(uint8_t *p, uint32_t v) {
    *p++ = (uint8_t)(v>>24); *p++ = (uint8_t)(v>>16);
    *p++ = (uint8_t)(v>>8);  *p++ = (uint8_t)v;
    return p;
}

static inline uint8_t * lh_place_long_be(uint8_t *p, uint64_t v) {
    *p++ = (uint8_t)(v>>56); *p++ = (uint8_t)(v>>48);
    *p++ = (uint8_t)(v>>40); *p++ = (uint8_t)(v>>32);
    *p++ = (uint8_t)(v>>24); *p++ = (uint8_t)(v>>16);
    *p++ = (uint8_t)(v>>8);  *p++ = (uint8_t)v;
    return p;
}

static inline uint8_t * lh_place_float_le(uint8_t *p, float v) {
    union { float f; uint32_t i; } temp;
    temp.f = v;
    return lh_place_int_le(p,temp.i);
}

static inline uint8_t * lh_place_float_be(uint8_t *p, float v) {
    union { float f; uint32_t i; } temp;
    temp.f = v;
    return lh_place_int_be(p,temp.i);
}

static inline uint8_t * lh_place_double_le(uint8_t *p, double v) {
    union { double f; uint64_t i; } temp;
    temp.f = v;
    return lh_place_long_le(p,temp.i);
}

static inline uint8_t * lh_place_double_be(uint8_t *p, double v) {
    union { double f; uint64_t i; } temp;
    temp.f = v;
    return lh_place_long_be(p,temp.i);
}

static inline uint8_t * lh_place_varint(uint8_t *p, uint32_t v) {
    do {
        *p++ = (v&0x7f)|0x80;
        v >>= 7;
    } while (v>0);
    *(p-1) &= 0x7f;
    return p;
}

#define lh_write_char(ptr,val)          ptr=lh_place_char(ptr,val)
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

#define def_lwrite(name,type)                                           \
    static inline int _lh_lwrite_##name(uint8_t **p, uint8_t *l, type v) { \
        if (l<*p+sizeof(v)) return 0;                                   \
        lh_write_##name(*p,v);                                          \
        return 1;                                                       \
    }

def_lwrite(char,uint8_t);
def_lwrite(short_be,uint16_t);
def_lwrite(short_le,uint16_t);
def_lwrite(int_be,uint32_t);
def_lwrite(int_le,uint32_t);
def_lwrite(long_be,uint64_t);
def_lwrite(long_le,uint64_t);
def_lwrite(float_be,float);
def_lwrite(float_le,float);
def_lwrite(double_be,double);
def_lwrite(double_le,double);

static inline int _lh_lwrite_varint(uint8_t **p, uint8_t *l, uint32_t v) {
    uint8_t *temp = *p;
    do {
        if (*p>=l) {
            *p = temp;
            return 0;
        }
        *(*p)++ = (v&0x7f)|0x80;
        v >>= 7;
    } while (v>0);
    *((*p)-1) &= 0x7f;
    return 1;
}

#define lh_lwrite_char(p,l,v)       _lh_lwrite_char(&p,l,v)
#define lh_lwrite_short_be(p,l,v)   _lh_lwrite_short_be(&p,l,v)
#define lh_lwrite_short_le(p,l,v)   _lh_lwrite_short_le(&p,l,v)
#define lh_lwrite_int_be(p,l,v)     _lh_lwrite_int_be(&p,l,v)
#define lh_lwrite_int_le(p,l,v)     _lh_lwrite_int_le(&p,l,v)
#define lh_lwrite_long_be(p,l,v)    _lh_lwrite_long_be(&p,l,v)
#define lh_lwrite_long_le(p,l,v)    _lh_lwrite_long_le(&p,l,v)
#define lh_lwrite_float_be(p,l,v)   _lh_lwrite_float_be(&p,l,v)
#define lh_lwrite_float_le(p,l,v)   _lh_lwrite_float_le(&p,l,v)
#define lh_lwrite_double_be(p,l,v)  _lh_lwrite_double_be(&p,l,v)
#define lh_lwrite_double_le(p,l,v)  _lh_lwrite_double_le(&p,l,v)
#define lh_lwrite_varint(p,l,v)     _lh_lwrite_varint(&p,l,v)

////////////////////////////////////////////////////////////////////////////////

static inline int lh_varint_size(uint32_t v) {
    int size = 0;
    do {
        v>>=7;
        size++;
    } while (v>0);
    return size;
}

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

#define lh_parse_char_be        lh_parse_char
#define lh_parse_char_le        lh_parse_char
#define lh_read_char_be         lh_read_char
#define lh_read_char_le         lh_read_char
#define lh_lread_char_be        lh_lread_char
#define lh_lread_char_le        lh_lread_char

#define lh_place_char_be        lh_place_char
#define lh_place_char_le        lh_place_char
#define lh_write_char_be        lh_write_char
#define lh_write_char_le        lh_write_char
#define lh_lwrite_char_be       lh_lwrite_char
#define lh_lwrite_char_le       lh_lwrite_char


#ifdef LH_DECLARE_SHORT_NAMES

#define bswap_short             lh_bswap_short
#define bswap_int               lh_bswap_int
#define bswap_long              lh_bswap_long
#define bswap_float             lh_bswap_float
#define bswap_double            lh_bswap_double

#define ibswap_short            lh_ibswap_short
#define ibswap_int              lh_ibswap_int
#define ibswap_long             lh_ibswap_long
#define ibswap_float            lh_ibswap_float
#define ibswap_double           lh_ibswap_double

#define parse_char              lh_parse_char
#define parse_char_le           lh_parse_char
#define parse_short             lh_parse_short_be
#define parse_short_le          lh_parse_short_le
#define parse_int               lh_parse_int_be
#define parse_int_le            lh_parse_int_le
#define parse_long              lh_parse_long_be
#define parse_long_le           lh_parse_long_le
#define parse_float             lh_parse_float_be
#define parse_float_le          lh_parse_float_le
#define parse_double            lh_parse_double_be
#define parse_double_le         lh_parse_double_le
#define parse_varint            lh_parse_varint

#define read_char               lh_read_char
#define read_char_le            lh_read_char
#define read_short              lh_read_short_be
#define read_short_le           lh_read_short_le
#define read_int                lh_read_int_be
#define read_int_le             lh_read_int_le
#define read_long               lh_read_long_be
#define read_long_le            lh_read_long_le
#define read_float              lh_read_float_be
#define read_float_le           lh_read_float_le
#define read_double             lh_read_double_be
#define read_double_le          lh_read_double_le

#define lread_char              lh_lread_char
#define lread_char_le           lh_lread_char
#define lread_short             lh_lread_short_be
#define lread_short_le          lh_lread_short_le
#define lread_int               lh_lread_int_be
#define lread_int_le            lh_lread_int_le
#define lread_long              lh_lread_long_be
#define lread_long_le           lh_lread_long_le
#define lread_float             lh_lread_float_be
#define lread_float_le          lh_lread_float_le
#define lread_double            lh_lread_double_be
#define lread_double_le         lh_lread_double_le

#define place_char              lh_place_char
#define place_char_le           lh_place_char
#define place_short             lh_place_short_be
#define place_short_le          lh_place_short_le
#define place_int               lh_place_int_be
#define place_int_le            lh_place_int_le
#define place_long              lh_place_long_be
#define place_long_le           lh_place_long_le
#define place_float             lh_place_float_be
#define place_float_le          lh_place_float_le
#define place_double            lh_place_double_be
#define place_double_le         lh_place_double_le
#define place_varint            lh_place_varint

#define write_char              lh_write_char
#define write_char_le           lh_write_char
#define write_short             lh_write_short_be
#define write_short_le          lh_write_short_le
#define write_int               lh_write_int_be
#define write_int_le            lh_write_int_le
#define write_long              lh_write_long_be
#define write_long_le           lh_write_long_le
#define write_float             lh_write_float_be
#define write_float_le          lh_write_float_le
#define write_double            lh_write_double_be
#define write_double_le         lh_write_double_le
#define write_varint            lh_write_varint

#define varint_size             lh_varint_size

#endif
