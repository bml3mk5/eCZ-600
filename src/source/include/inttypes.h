/* this file is simple inttypes.h header. */

#ifndef _SIMPLE_INTTYPES_H_
#define _SIMPLE_INTTYPES_H_

#if defined(_MSC_VER) && _MSC_VER <= 1500

#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef int16_t
typedef signed short int16_t;
#endif
#ifndef uint16_t
typedef unsigned short uint16_t;
#endif
#ifndef int32_t
typedef signed int int32_t;
#endif
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif
#ifndef int64_t
typedef signed __int64 int64_t;
#endif
#ifndef uint64_t
typedef unsigned __int64 uint64_t;
#endif

#endif /* _MSC_VER <= 1500 */

#endif /* _SIMPLE_INTTYPES_H_ */
