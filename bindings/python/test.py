import sys
import edlib

testFailed = False

result = edlib.align("telephone", "elephant")

if not (result and result["editDistance"] == 3):
    testFailed = True

if testFailed:
    print("Some of the tests failed!")
else:
    print("All tests passed!")

sys.exit(testFailed)
