#include "edlib.h"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

namespace {
using Clock = std::chrono::steady_clock;

int number(const char* text, int minimum, int maximum) {
    std::size_t end = 0;
    const long value = std::stol(text, &end);
    if (text[end] != '\0' || value < minimum || value > maximum) {
        throw std::runtime_error("numeric argument out of range");
    }
    return static_cast<int>(value);
}

std::uint64_t fingerprint(const std::string& query, const std::string& target) {
    std::uint64_t hash = UINT64_C(14695981039346656037);
    for (const auto* sequence : {&query, &target}) {
        for (unsigned char c : *sequence) {
            hash = (hash ^ c) * UINT64_C(1099511628211);
        }
        hash = (hash ^ sequence->size()) * UINT64_C(1099511628211);
    }
    return hash;
}

std::uint64_t peakRssBytes() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters{};
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters))) {
        throw std::runtime_error("GetProcessMemoryInfo failed");
    }
    return counters.PeakWorkingSetSize;
#else
    rusage usage{};
    if (getrusage(RUSAGE_SELF, &usage) != 0) {
        throw std::runtime_error("getrusage failed");
    }
#ifdef __APPLE__
    return static_cast<std::uint64_t>(usage.ru_maxrss);
#else
    return static_cast<std::uint64_t>(usage.ru_maxrss) * 1024;
#endif
#endif
}

// Use raw mt19937 output, not implementation-dependent distributions or shuffle.
// Mutations mix substitutions, insertions and deletions. The requested target
// length is its length before these mutations, not the final length.
void sequences(int queryLength, int targetLength, int alphabet, int mutationPercent,
               EdlibAlignMode mode, unsigned seed, std::string& query, std::string& target) {
    std::mt19937 random(seed);
    query.resize(queryLength);
    for (char& c : query) c = static_cast<char>(random() % alphabet);
    std::string segment;
    segment.reserve(queryLength * 2);
    for (unsigned char c : query) {
        if (random() % 100 >= static_cast<unsigned>(mutationPercent)) {
            segment.push_back(static_cast<char>(c));
            continue;
        }
        const unsigned operation = random() % 3;
        const char different = static_cast<char>((c + 1 + random() % (alphabet - 1)) % alphabet);
        if (operation == 0) segment.push_back(different);
        if (operation == 1) {
            segment.push_back(different);
            segment.push_back(static_cast<char>(c));
        }
        // operation == 2 deletes the source character.
    }
    const int padding = targetLength - queryLength;
    const int prefix = mode == EDLIB_MODE_HW ? padding / 2 : 0;
    target.reserve(segment.size() + padding);
    for (int i = 0; i < prefix; ++i) target.push_back(static_cast<char>(random() % alphabet));
    target += segment;
    for (int i = prefix; i < padding; ++i) target.push_back(static_cast<char>(random() % alphabet));
    const char* symbols = alphabet == 4 ? "ACGT" : alphabet == 20 ? "ACDEFGHIKLMNPQRSTVWY" : nullptr;
    if (symbols) {
        for (char& c : query) c = symbols[static_cast<unsigned char>(c)];
        for (char& c : target) c = symbols[static_cast<unsigned char>(c)];
    }
}

int distance(const std::string& query, const std::string& target, EdlibAlignConfig config) {
    EdlibAlignResult result = edlibAlign(query.data(), static_cast<int>(query.size()),
                                       target.data(), static_cast<int>(target.size()), config);
    const int score = result.editDistance;
    const int status = result.status;
    edlibFreeAlignResult(result);
    if (status != EDLIB_STATUS_OK) throw std::runtime_error("edlibAlign failed");
    return score;
}
} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 11) {
            std::cerr << "Usage: edlib-benchmark QUERY_LENGTH TARGET_LENGTH ALPHABET MUTATION_PERCENT "
                         "NW|SHW|HW distance|locations|path auto|bounded|reject SEED SAMPLES SAMPLE_MS\n"
                         "Run benchmarks/run.py for the complete matrix.\n";
            return 1;
        }
        const int queryLength = number(argv[1], 1, 1000000);
        const int targetLength = number(argv[2], queryLength, 10000000);
        const int alphabet = number(argv[3], 2, 256);
        const int mutationPercent = number(argv[4], 0, 100);
        const std::string modeName = argv[5], taskName = argv[6], cutoff = argv[7];
        EdlibAlignMode mode;
        if (modeName == "NW") mode = EDLIB_MODE_NW;
        else if (modeName == "SHW") mode = EDLIB_MODE_SHW;
        else if (modeName == "HW") mode = EDLIB_MODE_HW;
        else throw std::runtime_error("unknown alignment mode");
        EdlibAlignTask task;
        if (taskName == "distance") task = EDLIB_TASK_DISTANCE;
        else if (taskName == "locations") task = EDLIB_TASK_LOC;
        else if (taskName == "path") task = EDLIB_TASK_PATH;
        else throw std::runtime_error("unknown alignment task");
        if (cutoff != "auto" && cutoff != "bounded" && cutoff != "reject") {
            throw std::runtime_error("unknown cutoff policy");
        }
        const unsigned seed = static_cast<unsigned>(number(argv[8], 0, std::numeric_limits<int>::max()));
        const int samples = number(argv[9], 1, 1000);
        const int sampleMs = number(argv[10], 1, 60000);
        std::string query, target;
        sequences(queryLength, targetLength, alphabet, mutationPercent, mode, seed, query, target);
        const int reference = distance(query, target,
            edlibNewAlignConfig(-1, mode, EDLIB_TASK_DISTANCE, nullptr, 0));
        if (reference < 0) throw std::runtime_error("unbounded alignment returned no solution");
        const int k = cutoff == "auto" ? -1 : cutoff == "bounded" ? reference : reference - 1;
        const bool skipped = cutoff == "reject" && reference == 0;
        const int expected = cutoff == "reject" ? -1 : reference;
        const auto config = edlibNewAlignConfig(k, mode, task, nullptr, 0);
        std::vector<double> timings;
        timings.reserve(samples);
        std::uint64_t iterations = 1;
        if (!skipped) {
            auto batch = [&](std::uint64_t count) {
                const auto start = Clock::now();
                for (std::uint64_t i = 0; i < count; ++i) {
                    if (distance(query, target, config) != expected) {
                        throw std::runtime_error("task/cutoff result disagrees with unbounded distance");
                    }
                }
                return std::chrono::duration<double, std::nano>(Clock::now() - start).count();
            };
            batch(1); // Warm up the selected task before calibration and measurement.
            const double minimumNs = sampleMs * 1000000.0;
            while (batch(iterations) < minimumNs && iterations < (UINT64_C(1) << 24)) {
                iterations *= 2;
            }
            for (int i = 0; i < samples; ++i) timings.push_back(batch(iterations) / iterations);
        }
        std::cout << std::setprecision(12)
                  << "{\"query_length\":" << query.size()
                  << ",\"target_length\":" << target.size()
                  << ",\"input_hash\":\"" << std::hex << fingerprint(query, target) << std::dec
                  << "\",\"reference_distance\":" << reference
                  << ",\"k\":" << k
                  << ",\"skipped\":" << (skipped ? "true" : "false")
                  << ",\"iterations\":" << (skipped ? 0 : iterations)
                  << ",\"peak_rss_bytes\":" << peakRssBytes()
                  << ",\"compiler\":\""
#if defined(__clang__)
                  << "clang " << __clang_major__ << '.' << __clang_minor__ << '.' << __clang_patchlevel__
#elif defined(__GNUC__)
                  << "gcc " << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__
#elif defined(_MSC_VER)
                  << "msvc " << _MSC_VER
#else
                  << "unknown"
#endif
                  << "\",\"samples_ns\":[";
        for (std::size_t i = 0; i < timings.size(); ++i) {
            if (i) std::cout << ',';
            std::cout << timings[i];
        }
        std::cout << "]}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}
