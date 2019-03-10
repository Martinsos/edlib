cimport cython
from cpython.mem cimport PyMem_Malloc, PyMem_Free

cimport cedlib


def align(query, target, mode="NW", task="distance", k=-1, additionalEqualities=None):
    """ Align query with target using edit distance.
    @param {str or bytes or iterable of hashable objects} query
    @param {str or bytes or iterable of hashable objects} target
    @param {string} mode  Optional. Alignment method do be used. Possible values are:
            - 'NW' for global (default)
            - 'HW' for infix
            - 'SHW' for prefix.
    @param {string} task  Optional. Tells edlib what to calculate. Less there is to calculate,
            faster it is. Possible value are (from fastest to slowest):
            - 'distance' - find edit distance and end locations in target. Default.
            - 'locations' - find edit distance, end locations and start locations.
            - 'path' - find edit distance, start and end locations and alignment path.
    @param {int} k  Optional. Max edit distance to search for - the lower this value,
            the faster is calculation. Set to -1 (default) to have no limit on edit distance.
    @param {list} additionalEqualities  Optional.
            List of pairs of characters, where each pair defines two characters as equal.
            This way you can extend edlib's definition of equality (which is that each character is equal only
            to itself).
            This can be useful e.g. when you want edlib to be case insensitive, or if you want certain
            characters to act as a wildcards.
            Set to None (default) if you do not want to extend edlib's default equality definition.
    @return Dictionary with following fields:
            {int} editDistance  -1 if it is larger than k.
            {int} alphabetLength
            {[(int, int)]} locations  List of locations, in format [(start, end)].
            {string} cigar  Cigar is a standard format for alignment path.
                Here we are using extended cigar format, which uses following symbols:
                Match: '=', Insertion to target: 'I', Deletion from target: 'D', Mismatch: 'X'.
                e.g. cigar of "5=1X1=1I" means "5 matches, 1 mismatch, 1 match, 1 insertion (to target)".
    """
    # Transform python sequences of hashables into c strings.
    cdef bytes query_bytes
    cdef bytes target_bytes
    query_needs_alphabet = False
    target_needs_alphabet = False
    if isinstance(query, bytes):
        query_bytes = query
    elif isinstance(query, str):
        query_bytes = query.encode('utf-8')
        query_needs_alphabet = len(query_bytes) != len(query)
    else:
        query_needs_alphabet = True

    if isinstance(target, bytes):
        target_bytes = target
    elif isinstance(target, str):
        target_bytes = target.encode('utf-8')
        target_needs_alphabet = len(target_bytes) != len(target)
    else:
        target_needs_alphabet = True

    # Map non-ascii symbols into an ASCII alphabet so it can be used
    # in the C++ code
    alphabet = {}
    if query_needs_alphabet or target_needs_alphabet:
        query_vals = set(query)
        target_vals = set(target)
        alphabet = {c: chr(x) for x, c in enumerate(query_vals.union(target_vals))}
        if len(alphabet) > 255:
            raise ValueError(
                "query and target combined have more than 255 unique values, "
                "this is not supported.")
    if query_needs_alphabet:
        query_bytes = ''.join(alphabet[c] for c in query).encode('ascii')
    if target_needs_alphabet:
        target_bytes = ''.join(alphabet[c] for c in target).encode('ascii')

    cdef char* cquery = query_bytes;
    cdef char* ctarget = target_bytes;

    # Build an edlib config object based on given parameters.
    cconfig = cedlib.edlibDefaultAlignConfig()

    if k is not None: cconfig.k = k

    if mode == 'NW': cconfig.mode = cedlib.EDLIB_MODE_NW
    if mode == 'HW': cconfig.mode = cedlib.EDLIB_MODE_HW
    if mode == 'SHW': cconfig.mode = cedlib.EDLIB_MODE_SHW

    if task == 'distance': cconfig.task = cedlib.EDLIB_TASK_DISTANCE
    if task == 'locations': cconfig.task = cedlib.EDLIB_TASK_LOC
    if task == 'path': cconfig.task = cedlib.EDLIB_TASK_PATH

    cdef bytes tmp_bytes;
    cdef char* tmp_cstring;
    if additionalEqualities is None:
        cconfig.additionalEqualities = NULL
        cconfig.additionalEqualitiesLength = 0
    else:
        cconfig.additionalEqualities = <cedlib.EdlibEqualityPair*> PyMem_Malloc(len(additionalEqualities)
                                                                          * cython.sizeof(cedlib.EdlibEqualityPair))
        for i in range(len(additionalEqualities)):
            cconfig.additionalEqualities[i].first = bytearray(additionalEqualities[i][0].encode('utf-8'))[0]
            cconfig.additionalEqualities[i].second = bytearray(additionalEqualities[i][1].encode('utf-8'))[0]
        cconfig.additionalEqualitiesLength = len(additionalEqualities)

    # Run alignment -- need to get len before disabling the GIL
    query_len = len(query)
    target_len = len(target)
    with nogil:
        cresult = cedlib.edlibAlign(cquery, query_len, ctarget, target_len, cconfig)
    if cconfig.additionalEqualities != NULL: PyMem_Free(cconfig.additionalEqualities)

    if cresult.status == 1:
        raise Exception("There was an error.")

    # Build python dictionary with results from result object that edlib returned.
    locations = []
    if cresult.numLocations >= 0:
        for i in range(cresult.numLocations):
            locations.append((cresult.startLocations[i] if cresult.startLocations else None,
                              cresult.endLocations[i] if cresult.endLocations else None))
    cigar = None
    if cresult.alignment:
        ccigar = cedlib.edlibAlignmentToCigar(cresult.alignment, cresult.alignmentLength,
                                              cedlib.EDLIB_CIGAR_EXTENDED)
        cigar = <bytes> ccigar
        cigar = cigar.decode('UTF-8')
    result = {
        'editDistance': cresult.editDistance,
        'alphabetLength': cresult.alphabetLength,
        'locations': locations,
        'cigar': cigar
    }
    cedlib.edlibFreeAlignResult(cresult)

    return result
