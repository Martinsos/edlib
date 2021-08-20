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

# Empty characters.
result = edlib.align("", "elephant")
testFailed = testFailed or (not (result and result["editDistance"] == 8))
result = edlib.align("telephone", "")
testFailed = testFailed or (not (result and result["editDistance"] == 9))
result = edlib.align("", "elephant", mode="HW")
testFailed = testFailed or (not (result and result["editDistance"] == 0))
result = edlib.align("telephone", "", mode="HW")
testFailed = testFailed or (not (result and result["editDistance"] == 9))
result = edlib.align("", "elephant", mode="SHW")
testFailed = testFailed or (not (result and result["editDistance"] == 0))
result = edlib.align("telephone", "", mode="SHW")
testFailed = testFailed or (not (result and result["editDistance"] == 9))

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
