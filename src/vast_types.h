#ifndef VAST_TYPES_H
#define VAST_TYPES_H

/* #############################################################################
 * # [SECTION] Compiler Specific Settings
 * #############################################################################
 */

/* Inline */
#if __STDC_VERSION__ >= 199901L
#define VAST_INLINE inline
#elif defined(__GNUC__) || defined(__clang__)
#define VAST_INLINE __inline__
#elif defined(_MSC_VER)
#define VAST_INLINE __inline
#else
#define VAST_INLINE
#endif

/* Alignment */
#if defined(_MSC_VER)
#define VAST_ALIGN(x) __declspec(align(x))
#elif defined(__GNUC__) || defined(__clang__)
#define VAST_ALIGN(x) __attribute__((aligned(x)))
#else
#define VAST_ALIGN(x)
#endif

/* Architecture */
#if defined(__x86_64__) || defined(_M_X64)
#define VAST_ARCH_X64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define VAST_ARCH_ARM64
#endif

/* API */
#define VAST_API static
#define VAST_NULL (void *)0

/* #############################################################################
 * # [SECTION] Basic Types
 * #############################################################################
 */
typedef char s8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef short i16;
typedef int i32;
typedef float f32;
typedef double f64;

#define VAST_TYPES_STATIC_ASSERT(c, m) typedef char vast_types_assert_##m[(c) ? 1 : -1]
VAST_TYPES_STATIC_ASSERT(sizeof(s8) == 1, s8_size_must_be_1);
VAST_TYPES_STATIC_ASSERT(sizeof(u8) == 1, u8_size_must_be_1);
VAST_TYPES_STATIC_ASSERT(sizeof(u16) == 2, u16_size_must_be_2);
VAST_TYPES_STATIC_ASSERT(sizeof(i16) == 2, i16_size_must_be_2);
VAST_TYPES_STATIC_ASSERT(sizeof(u32) == 4, u32_size_must_be_4);
VAST_TYPES_STATIC_ASSERT(sizeof(i32) == 4, i32_size_must_be_4);
VAST_TYPES_STATIC_ASSERT(sizeof(f32) == 4, f32_size_must_be_4);
VAST_TYPES_STATIC_ASSERT(sizeof(f64) == 8, f64_size_must_be_8);
#undef VAST_TYPES_STATIC_ASSERT

#endif /* VAST_TYPES_H */