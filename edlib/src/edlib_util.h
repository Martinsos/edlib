#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <atomic>
#if defined(_MSC_VER) && defined(EDLIB_X86_SIMD)
#include <malloc.h>
#endif

#include "simd_util.h"

static const int MAX_UCHAR = 255;

enum TransformedSequenceType {
    EDLIB_TST_MALLOC,
    EDLIB_TST_ALIGNED_OFFSET_ALLOC,
};

struct TransformedSequencePointer {
    TransformedSequenceType type;
    unsigned char* data;
};

// Utility functions for the SIMD path:

#ifdef EDLIB_X86_SIMD

/** Perform 32 lookups in a 256 byte table. Table must be 32 byte aligned. */
static __m256i lookup256ByteTableAVX2(const unsigned char* table,
        __m256i idx) {
    __m256i res_total = _mm256_set1_epi8(0);
    // TODO fix, only does first 128b
    for (int ii = 0; ii < 16; ++ii) {
        __m256i idx16 = _mm256_sub_epi8(idx, _mm256_set1_epi8(static_cast<unsigned char>(ii * 16)));
        idx16 = _mm256_adds_epu8(idx16, _mm256_set1_epi8(0x70));
        __m256i lut = _mm256_broadcastsi128_si256(
                _mm_load_si128(reinterpret_cast<const __m128i*>(table + ii * 16)));
        __m256i res = _mm256_shuffle_epi8(lut, idx16);
        res_total = _mm256_or_si256(res_total, res);
    }
    return res_total;
}

/**
 * Perform 32 lookups in a 96 byte table stored in 6 registers.
 *
 * This is the same as lookup256ByteTableAVX2, but the table is small enough to
 * fit into registers.
 */
static inline __m256i lookup6RegTableAVX2(__m256i lut0, __m256i lut1, __m256i lut2,
        __m256i lut3, __m256i lut4, __m256i lut5,
        __m256i idx) {
    __m256i idx16;
    __m256i sub_mask;
    __m256i add_mask = _mm256_set1_epi8(0x70);
    __m256i res_total = _mm256_set1_epi8(0);

    sub_mask = _mm256_set1_epi8(0 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut0, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    sub_mask = _mm256_set1_epi8(1 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut1, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    sub_mask = _mm256_set1_epi8(2 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut2, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    sub_mask = _mm256_set1_epi8(3 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut3, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    sub_mask = _mm256_set1_epi8(4 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut4, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    sub_mask = _mm256_set1_epi8(5 * 16);
    idx16 = _mm256_sub_epi8(idx, sub_mask);
    idx16 = _mm256_adds_epu8(idx16, add_mask);
    idx16 = _mm256_shuffle_epi8(lut5, idx16);
    res_total = _mm256_or_si256(res_total, idx16);

    return res_total;
}

/**
 * Checks whether every byte in 4 YMM vectors is contains in the top-exclusive
 * range min..max
 */
static inline bool allBytesInRange4AVX2(__m256i v0, __m256i v1, __m256i v2, __m256i v3, uint8_t min, uint8_t max) {
    __m256i val_min = _mm256_set1_epi8(min);
    __m256i val_max = _mm256_set1_epi8(max-1);
    __m256i any_out_of_range, lt, gt;
    lt = _mm256_cmpgt_epi8(val_min, v0);
    any_out_of_range = lt;
    gt = _mm256_cmpgt_epi8(v0, val_max);
    any_out_of_range = _mm256_or_si256(any_out_of_range, gt);
    lt = _mm256_cmpgt_epi8(val_min, v1);
    any_out_of_range = _mm256_or_si256(any_out_of_range, lt);
    gt = _mm256_cmpgt_epi8(v1, val_max);
    any_out_of_range = _mm256_or_si256(any_out_of_range, gt);
    lt = _mm256_cmpgt_epi8(val_min, v2);
    any_out_of_range = _mm256_or_si256(any_out_of_range, lt);
    gt = _mm256_cmpgt_epi8(v2, val_max);
    any_out_of_range = _mm256_or_si256(any_out_of_range, gt);
    lt = _mm256_cmpgt_epi8(val_min, v3);
    any_out_of_range = _mm256_or_si256(any_out_of_range, lt);
    gt = _mm256_cmpgt_epi8(v3, val_max);
    any_out_of_range = _mm256_or_si256(any_out_of_range, gt);
    return static_cast<uint32_t>(_mm256_movemask_epi8(any_out_of_range)) != 0;
}


/**
 * Checks whether every byte in 4 YMM vectors is contained in `set[0..count]`
 */
static inline bool allBytesInSet4AVX2(__m256i v0, __m256i v1, __m256i v2, __m256i v3, const uint8_t* set, size_t count) {
    __m256i match_any_0 = _mm256_setzero_si256();
    __m256i match_any_1 = _mm256_setzero_si256();
    __m256i match_any_2 = _mm256_setzero_si256();
    __m256i match_any_3 = _mm256_setzero_si256();
    for (size_t i = 0; i < count; i++) {
        __m256i cmp, val;
        val = _mm256_set1_epi8(static_cast<char>(set[i]));
        cmp = _mm256_cmpeq_epi8(v0, val);
        match_any_0 = _mm256_or_si256(match_any_0, cmp);
        cmp = _mm256_cmpeq_epi8(v1, val);
        match_any_1 = _mm256_or_si256(match_any_1, cmp);
        cmp = _mm256_cmpeq_epi8(v2, val);
        match_any_2 = _mm256_or_si256(match_any_2, cmp);
        cmp = _mm256_cmpeq_epi8(v3, val);
        match_any_3 = _mm256_or_si256(match_any_3, cmp);
    }
    return
        static_cast<uint32_t>(_mm256_movemask_epi8(match_any_0)) == 0xFFFFFFFFu &&
        static_cast<uint32_t>(_mm256_movemask_epi8(match_any_1)) == 0xFFFFFFFFu &&
        static_cast<uint32_t>(_mm256_movemask_epi8(match_any_2)) == 0xFFFFFFFFu &&
        static_cast<uint32_t>(_mm256_movemask_epi8(match_any_3)) == 0xFFFFFFFFu;
}

#endif // EDLIB_X86_SIMD

/** Convert the inAlphabet bool vector to a list of indices */
static inline void alphabetToIndices(const bool* inAlphabet, std::vector<uint8_t>& out) {
    out.clear();
    for (int i = 0; i <= MAX_UCHAR; i++) {
        if (inAlphabet[i]) {
            out.push_back(static_cast<uint8_t>(i));
        }
    }
}

using TransformSequencesFunc = std::string (*)(const char*, int, const char*, int,
                                               TransformedSequencePointer*, TransformedSequencePointer*);
static std::atomic<TransformSequencesFunc> g_transformSequencesFunc{nullptr};

static std::string transformSequencesScalar(const char* const queryOriginal, const int queryLength,
                                 const char* const targetOriginal, const int targetLength,
                                 TransformedSequencePointer* const queryTransformed_,
                                 TransformedSequencePointer* const targetTransformed_) {
    // Alphabet is constructed from letters that are present in sequences.
    // Each letter is assigned an ordinal number, starting from 0 up to alphabetLength - 1,
    // and new query and target are created in which letters are replaced with their ordinal numbers.
    // This query and target are used in all the calculations later.

    unsigned char *queryTransformed = static_cast<unsigned char *>(malloc(sizeof(unsigned char) * queryLength));
    unsigned char *targetTransformed = static_cast<unsigned char *>(malloc(sizeof(unsigned char) * targetLength));

    char alphabet[MAX_UCHAR + 1];
    int alphabetSize = 0;

    // Alphabet information, it is constructed on fly while transforming sequences.
    // letterIdx[c] is index of letter c in alphabet.
    unsigned char letterIdx[MAX_UCHAR + 1];
    bool inAlphabet[MAX_UCHAR + 1]; // inAlphabet[c] is true if c is in alphabet
    for (int i = 0; i < MAX_UCHAR + 1; i++) inAlphabet[i] = false;

    for (int i = 0; i < queryLength; i++) {
        unsigned char c = static_cast<unsigned char>(queryOriginal[i]);
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
            letterIdx[c] = idx;
            alphabet[idx] = queryOriginal[i];
        }
        queryTransformed[i] = letterIdx[c];
    }

    for (int i = 0; i < targetLength; i++) {
        unsigned char c = static_cast<unsigned char>(targetOriginal[i]);
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
            letterIdx[c] = idx;
            alphabet[idx] = targetOriginal[i];
        }
        targetTransformed[i] = letterIdx[c];
    }


    queryTransformed_->type = EDLIB_TST_MALLOC;
    queryTransformed_->data = queryTransformed;
    targetTransformed_->type = EDLIB_TST_MALLOC;
    targetTransformed_->data = targetTransformed;

    return std::string(alphabet, alphabetSize);
}

/** SIMD version of transformSequencesScalar */
static std::string transformSequencesAVX2(const char* const queryOriginal, const int queryLength,
                                 const char* const targetOriginal, const int targetLength,
                                 TransformedSequencePointer* const queryTransformed_,
                                 TransformedSequencePointer* const targetTransformed_) {
#ifdef EDLIB_X86_SIMD

    if (targetLength < 1500) {
        return transformSequencesScalar(queryOriginal, queryLength,
                                        targetOriginal, targetLength,
                                        queryTransformed_, targetTransformed_);
    }

    // We use _mm_malloc to get aligned memory here - extra 32 bytes for the
    // target so we can offset the pointer to match incoming target (see below)
    unsigned char *queryTransformed = static_cast<unsigned char *>(_mm_malloc(queryLength, 32));
    unsigned char *targetTransformed = static_cast<unsigned char *>(_mm_malloc(targetLength + 32, 32));

    // Offset targetTransformed such that it now has the *same alignment* as
    // targetOriginal. We added an extra 32 bytes onto the malloc for this
    // reason. This allows us to loop over both arrays and do aligned
    // loads/stores on both.
    targetTransformed += reinterpret_cast<uintptr_t>(targetOriginal) & 31;

    char alphabet[MAX_UCHAR + 1];
    int alphabetSize = 0;

    alignas(32) unsigned char letterIdx[MAX_UCHAR + 1];
    bool inAlphabet[MAX_UCHAR + 1]; // inAlphabet[c] is true if c is in alphabet
    for (int i = 0; i < MAX_UCHAR + 1; i++) inAlphabet[i] = false;

    // Transform query - this isn't vectorized, as it's typically short.
    for (int i = 0; i < queryLength; i++) {
        unsigned char c = static_cast<unsigned char>(queryOriginal[i]);
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
            letterIdx[c] = idx;
            alphabet[idx] = queryOriginal[i];
        }
        queryTransformed[i] = letterIdx[c];
    }

    // Input pointers aren't aligned, so 'skip' the first few elements until we're
    // aligned for the main SIMD loop. Here we compute how many to skip.
    int srcAlignOffset = (32 - (reinterpret_cast<uintptr_t>(targetOriginal) & 31)) & 31;
    int dstAlignOffset = srcAlignOffset;
    const char* toAligned = targetOriginal + srcAlignOffset;
    unsigned char* ttAligned = targetTransformed + dstAlignOffset;
    int alignedLen = ((targetLength - srcAlignOffset) / 128) * 128;

    // Do a mini scalar loop to cover the elements we 'skip'
    int prefixEnd = srcAlignOffset < targetLength ? srcAlignOffset : targetLength;
    for (int i = 0; i < prefixEnd; i++) {
        unsigned char c = static_cast<unsigned char>(targetOriginal[i]);
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
            letterIdx[c] = idx;
            alphabet[idx] = targetOriginal[i];
        }
        targetTransformed[i] = letterIdx[c];
    }

    // Lookup table stored in SIMD registers. This is the same as `letterIdx` in
    // the scalar loop, but only for chars between 32 <= c < 128. We only have
    // enough registers for these - if we encounter any 'weird' ascii chars
    // (e.g. <32 or >127), we have a slow loop to process those - but for almost
    // all cases we can keep the entire lut in registers
    __m256i lut2 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 2 * 16)));
    __m256i lut3 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 3 * 16)));
    __m256i lut4 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 4 * 16)));
    __m256i lut5 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 5 * 16)));
    __m256i lut6 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 6 * 16)));
    __m256i lut7 = _mm256_broadcastsi128_si256(
        _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 7 * 16)));

    // Transform the 'inAlphabet' set into a vector of indices for faster comparisons
    std::vector<uint8_t> inAlphabetSet;
    alphabetToIndices(inAlphabet, inAlphabetSet);

    // This is the main SIMD loop. process 32 elements per vector, and we unroll
    // the loop 4 times, so 128 elements per iteration.
    //
    // The 'fast path' of the loop
    for (int i = 0; i < alignedLen; i += 128) {
        __m256i idx0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(toAligned + i + 0 * 32));
        __m256i idx1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(toAligned + i + 1 * 32));
        __m256i idx2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(toAligned + i + 2 * 32));
        __m256i idx3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(toAligned + i + 3 * 32));

        // Slow path: check if there are any new alphabet chars. Since target is
        // large, this branch shouldn't happen often.
        //
        // Potential performance improvement: the inclusion of this path leads
        // to a slight 10% perf penalty in GCC - moving the slow path to outside
        // the loop & breakin ghere is faster on GCC, but slower on MSVC. A
        // future optimization would be figuring out a fast path for all
        // compilers.
        if (!allBytesInSet4AVX2(idx0, idx1, idx2, idx3, inAlphabetSet.data(), inAlphabetSet.size())) {
            alignas(32) unsigned char buf[128];
            _mm256_store_si256(reinterpret_cast<__m256i*>(buf + 0 * 32), idx0);
            _mm256_store_si256(reinterpret_cast<__m256i*>(buf + 1 * 32), idx1);
            _mm256_store_si256(reinterpret_cast<__m256i*>(buf + 2 * 32), idx2);
            _mm256_store_si256(reinterpret_cast<__m256i*>(buf + 3 * 32), idx3);
            for (int j = 0; j < 128; j++) {
                unsigned char c = buf[j];
                if (!inAlphabet[c]) {
                    inAlphabet[c] = true;
                    const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
                    letterIdx[c] = idx;
                    alphabet[idx] = static_cast<char>(c);
                }
            }
            alphabetToIndices(inAlphabet, inAlphabetSet);
            // Since the alphabet has changed, reload the LUT
            lut2 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 2 * 16)));
            lut3 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 3 * 16)));
            lut4 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 4 * 16)));
            lut5 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 5 * 16)));
            lut6 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 6 * 16)));
            lut7 = _mm256_broadcastsi128_si256(
                    _mm_load_si128(reinterpret_cast<const __m128i*>(letterIdx + 7 * 16)));
        }

        // Slow path: 'out-of-range' ascii chars which aren't
        // in the 6-register LUT range of 32-128.
        if (allBytesInRange4AVX2(idx0, idx1, idx2, idx3, 32, 128)) {
            __m256i res;
            res = lookup256ByteTableAVX2(letterIdx, idx0);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 0 * 32), res);
            res = lookup256ByteTableAVX2(letterIdx, idx1);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 1 * 32), res);
            res = lookup256ByteTableAVX2(letterIdx, idx2);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 2 * 32), res);
            res = lookup256ByteTableAVX2(letterIdx, idx3);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 3 * 32), res);
            continue;
        }

        // Subtract 32 from all values in the indices before lookup
        // (lookup6RegTableAVX2 expects input indices to be 0-96, possibly we
        // can fold this into lookup6RegTableAVX2 instead)
        {
            __m256i val_32 = _mm256_set1_epi8(32);
            idx0 = _mm256_sub_epi8(idx0, val_32);
            idx1 = _mm256_sub_epi8(idx1, val_32);
            idx2 = _mm256_sub_epi8(idx2, val_32);
            idx3 = _mm256_sub_epi8(idx3, val_32);
        }

        // Do the main fast path work - lookup the input chars in the alphabet
        // LUT to find their corresponding values, and write those corresponding
        // values into the output target array.
        __m256i res;
        res = lookup6RegTableAVX2(lut2, lut3, lut4, lut5, lut6, lut7, idx0);
        _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 0 * 32), res);
        res = lookup6RegTableAVX2(lut2, lut3, lut4, lut5, lut6, lut7, idx1);
        _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 1 * 32), res);
        res = lookup6RegTableAVX2(lut2, lut3, lut4, lut5, lut6, lut7, idx2);
        _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 2 * 32), res);
        res = lookup6RegTableAVX2(lut2, lut3, lut4, lut5, lut6, lut7, idx3);
        _mm256_stream_si256(reinterpret_cast<__m256i*>(ttAligned + i + 3 * 32), res);
    }

    // Clean up the end of the array, since SIMD loop only does 128 bytes at
    // once
    //
    // Potential performance improvement: more gradual tails, e.g. a 32 byte
    // SIMD tail loop before the scalar one
    int postfix_start = srcAlignOffset + alignedLen;
    for (int i = postfix_start; i < targetLength; i++) {
        unsigned char c = static_cast<unsigned char>(targetOriginal[i]);
        if (!inAlphabet[c]) {
            inAlphabet[c] = true;
            const unsigned char idx = static_cast<unsigned char>(alphabetSize++);
            letterIdx[c] = idx;
            alphabet[idx] = targetOriginal[i];
        }
        targetTransformed[i] = letterIdx[c];
    }

    queryTransformed_->type = EDLIB_TST_ALIGNED_OFFSET_ALLOC;
    queryTransformed_->data = queryTransformed;
    targetTransformed_->type = EDLIB_TST_ALIGNED_OFFSET_ALLOC;
    targetTransformed_->data = targetTransformed;

    return std::string(alphabet, alphabetSize);
#else
    abort();
#endif // EDLIB_X86_SIMD
}

/**
 * Takes char query and char target, recognizes alphabet and transforms them into unsigned char sequences
 * where elements in sequences are not any more letters of alphabet, but their index in alphabet.
 * Most of internal edlib functions expect such transformed sequences.
 * This function will allocate queryTransformed and targetTransformed, so make sure to free them when done.
 * Example:
 *   Original sequences: "ACT" and "CGT".
 *   Alphabet would be recognized as "ACTG". Alphabet length = 4.
 *   Transformed sequences: [0, 1, 2] and [1, 3, 2].
 * @param [in] queryOriginal
 * @param [in] queryLength
 * @param [in] targetOriginal
 * @param [in] targetLength
 * @param [out] queryTransformed  It will contain values in range [0, alphabet length - 1].
 * @param [out] targetTransformed  It will contain values in range [0, alphabet length - 1].
 * @return  Alphabet as a string of unique characters, where index of each character is its value in transformed
 *          sequences.
 */
static std::string transformSequences(
        const char* const queryOriginal, const int queryLength,
        const char* const targetOriginal, const int targetLength,
        TransformedSequencePointer* const queryTransformed_,
        TransformedSequencePointer* const targetTransformed_) {
    TransformSequencesFunc func = g_transformSequencesFunc.load(std::memory_order_relaxed);
    if (!func) {
        TransformSequencesFunc candidate = cpuSupportsAVX2() ? transformSequencesAVX2 : transformSequencesScalar;
        if (g_transformSequencesFunc.compare_exchange_strong(func, candidate, std::memory_order_relaxed)) {
            func = candidate;
        }
    }
    return func(queryOriginal, queryLength,
            targetOriginal, targetLength,
            queryTransformed_, targetTransformed_);
}

static void freeTransformedSequence(TransformedSequencePointer p) {
  switch (p.type) {
    case EDLIB_TST_MALLOC:
    free(p.data);
      break;
#ifdef EDLIB_X86_SIMD
    case EDLIB_TST_ALIGNED_OFFSET_ALLOC:
      // When we allocate aligned memory for the target array, we offset into
      // that array to match the source offset (see transformSequencesAVX2).
      // That means that `p.data` needs to be realigned to 32 bytes before
      // freeing.
      _mm_free(reinterpret_cast<void*>(
            (reinterpret_cast<size_t>(p.data) >> 5) << 5));
      break;
#endif
    default:
      abort();
  }
}
