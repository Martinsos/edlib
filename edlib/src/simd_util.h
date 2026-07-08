#pragma once

#include <cstdint>

// Define EDLIB_X86_SIMD when x86 SIMD intrinsics are available at compile time
// - so we can compile out SIMD x86 intrinsics on AArch64.
#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#define EDLIB_X86_SIMD
#include <immintrin.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#endif

/**
 * Returns true if the CPU supports AVX2 - returns false for AArch64 machines,
 * and x86_64 machines that don't support AVX2.
 */
static inline bool cpuSupportsAVX2() {
#ifdef EDLIB_X86_SIMD
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_cpu_supports("avx2");
#elif defined(_MSC_VER)
    int cpuInfo[4];
    __cpuidex(cpuInfo, 7, 0);
    return (cpuInfo[1] & (1 << 5)) != 0;
#else
    return false;
#endif
#else
    return false;
#endif
}
