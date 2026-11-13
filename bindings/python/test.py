import sys
import edlib

testFailed = False

result = edlib.align("telephone", "elephant")
if not (result and result["editDistance"] == 3):
    testFailed = True

result = edlib.align(b"telephone", b"elephant")
if not (result and result["editDistance"] == 3):
    testFailed = True

# Additional equalities.
result = edlib.align("ACTG", "CACTRT", mode="HW", task="path", additionalEqualities=[("R", "A"), ("R", "G")])
if not (result and result["editDistance"] == 0):
    testFailed = True

# Nice alignment.
resultNW = edlib.align(query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC", mode="NW", task="path")
test_getNiceNW = edlib.getNiceAlignment(resultNW, query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC")
if not (len(test_getNiceNW['query_aligned']) == 18 and len(test_getNiceNW['target_aligned'])==18):
    testFailed = True
if not (test_getNiceNW['query_aligned'] == 'TAAGGATGGTCCCAT-TC'):
        testFailed = True
if not (test_getNiceNW['matched_aligned'] == '-||||--||||.|||-||'):
        testFailed = True
if not (test_getNiceNW['target_aligned'] == '-AAGG--GGTCTCATATC'):
        testFailed = True

resultHW = edlib.align(query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC", mode="HW", task="path")
test_getNiceHW = edlib.getNiceAlignment(resultHW, query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC")
if not (len(test_getNiceHW['query_aligned']) == 18 and len(test_getNiceHW['target_aligned'])==18):
    testFailed = True
if not (test_getNiceHW['query_aligned'] == 'TAAGGATGGTCCCAT-TC'):
        testFailed = True
if not (test_getNiceHW['matched_aligned'] == '-||||--||||.|||-||'):
        testFailed = True
if not (test_getNiceHW['target_aligned'] == '-AAGG--GGTCTCATATC'):
        testFailed = True

resultSHW = edlib.align(query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC", mode="SHW", task="path")
test_getNiceSHW = edlib.getNiceAlignment(resultSHW, query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC")
if not (len(test_getNiceSHW['query_aligned']) == 18 and len(test_getNiceSHW['target_aligned'])==18):
    testFailed = True
if not (test_getNiceSHW['query_aligned'] == 'TAAGGATGGTCCCAT-TC'):
        testFailed = True
if not (test_getNiceSHW['matched_aligned'] == '-||||--||||.|||-||'):
        testFailed = True
if not (test_getNiceSHW['target_aligned'] == '-AAGG--GGTCTCATATC'):
        testFailed = True

result_taskDistance = edlib.align(query="TAAGGATGGTCCCATTC", target="AAGGGGTCTCATATC", mode="NW", task="distance")
if not (result_taskDistance["cigar"] == None):
    testFailed = True

# Empty inputs: distance limits, locations, CIGARs, and rendered paths.
empty_cases = [
    ("", "ACGT", "NW", 4, [(0, 3)], "4D", "----", "ACGT"),
    ("ACGT", "", "NW", 4, [(0, -1)], "4I", "ACGT", "----"),
    ("", "", "NW", 0, [(0, -1)], "", "", ""),
    ("", "ACGT", "SHW", 0, [(0, -1)], "", "", ""),
    ("ACGT", "", "SHW", 4, [(0, -1)], "4I", "ACGT", "----"),
    ("", "", "SHW", 0, [(0, -1)], "", "", ""),
    ("", "ACGT", "HW", 0, [(0, -1), (1, 0), (2, 1), (3, 2), (4, 3)], "", "", ""),
    ("ACGT", "", "HW", 4, [(0, -1)], "4I", "ACGT", "----"),
    ("", "", "HW", 0, [(0, -1)], "", "", ""),
]
for query, target, mode, score, locations, cigar, query_aligned, target_aligned in empty_cases:
    for task in ("distance", "locations", "path"):
        for k in (-1, 0, 3, 4, 5):
            result = edlib.align(query, target, mode=mode, task=task, k=k)
            within_limit = k < 0 or score <= k
            expected_locations = locations if within_limit else []
            if task == "distance":
                expected_locations = [(None, end) for _, end in expected_locations]
            expected = {
                "editDistance": score if within_limit else -1,
                "alphabetLength": 4 if query or target else 0,
                "locations": expected_locations,
                "cigar": cigar if task == "path" and within_limit else None,
            }
            if result != expected:
                print("Empty-input failure:", query, target, mode, task, k, result, expected)
                testFailed = True
            if task == "path" and within_limit:
                nice = edlib.getNiceAlignment(result, query, target)
                if nice != {
                    "query_aligned": query_aligned,
                    "matched_aligned": "-" * len(query_aligned),
                    "target_aligned": target_aligned,
                }:
                    print("Empty-input rendering failure:", query, target, mode, nice)
                    testFailed = True

# Unicode characters
result = edlib.align("ты милая", "ты гений")
testFailed = testFailed or (not (result and result["editDistance"] == 5 and result["alphabetLength"] == 12))

# Long alphabet.
long_alphabet = ''.join([chr(idx) for idx in range(1, 257)])
long_seq1 = long_alphabet * 3
long_seq2 = long_alphabet + long_alphabet[::-1] + long_alphabet
result = edlib.align(long_seq1, long_seq2)
testFailed = testFailed or (not (result and result["editDistance"] == 256))

if testFailed:
    print("Some of the tests failed!")
else:
    print("All tests passed!")

sys.exit(testFailed)
