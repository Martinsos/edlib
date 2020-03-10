cdef extern from "edlibGeneric.h" nogil:
     ctypedef enum EdlibAlignMode: EDLIB_MODE_NW, EDLIB_MODE_SHW, EDLIB_MODE_HW
     ctypedef enum EdlibAlignTask: EDLIB_TASK_DISTANCE, EDLIB_TASK_LOC, EDLIB_TASK_PATH
     ctypedef enum EdlibCigarFormat: EDLIB_CIGAR_STANDARD, EDLIB_CIGAR_EXTENDED

     char* edlibAlignmentToCigar(const unsigned char* alignment, int alignmentLength, EdlibCigarFormat cigarFormat)
     void edlibFreeAlignResult(EdlibAlignResult result)

     ctypedef struct EdlibAlignResult:
         int status
         int editDistance
         int* endLocations
         int* startLocations
         int numLocations
         unsigned char* alignment
         int alignmentLength
         int alphabetLength

cdef extern from "edlibGeneric.h" namespace "edlibGeneric" nogil:

     cdef cppclass EdlibEqualityPair[T]:
         int first
         int second

     cdef cppclass EdlibAlignConfig[T]:
         int k
         EdlibAlignMode mode
         EdlibAlignTask task
         EdlibEqualityPair[T]* additionalEqualities
         int additionalEqualitiesLength

     EdlibAlignConfig[T] edlibNewAlignConfig[T](int k, EdlibAlignMode mode, EdlibAlignTask task,
                                          EdlibEqualityPair[T]* additionalEqualities,
                                          int additionalEqualitiesLength)
     EdlibAlignConfig[T] edlibDefaultAlignConfig[T]()

     EdlibAlignResult edlibAlign[T](const T* query, int queryLength,
                                 const T* target, int targetLength,
                                 const EdlibAlignConfig config)

