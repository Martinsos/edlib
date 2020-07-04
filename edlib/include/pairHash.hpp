#include <functional>
// This header file provides a hashing function for std::pair of any custom types.
// Since there is no specialization of std::hash for std::pair<T1,T2> provided
// in the standard library, we can use the implementation in this header file.


/**
 * Takes a seed and a value of any type. A Seed can be a hash code of other
 * values or simply 0. It will generate a new hash code using the given seed and value.
 * It can be called repeatedly to incrementally create a hash value from several variables.
 * This "magical" implementation is adopted from Boost C++ Libraries.
 * https://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html
 * @param [in] seed The hash code of previous variables
 * @param [in] v The value to be hashed using the given seed
 */
template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename S, typename T> struct pair_hash
{
    inline std::size_t operator()(const std::pair<S, T> & v) const
    {
        std::size_t seed = 0;
        hash_combine(seed, v.first);
        hash_combine(seed, v.second);
        return seed;
    }
};