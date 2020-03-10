# distutils: language = c++
cimport cython
from cpython.mem cimport PyMem_Malloc, PyMem_Free
from libc.stdlib cimport malloc, free
import re

cimport cedlib

cdef map_to_int_array(query, target, additional_equalities, int query_len, int target_len, int* cquery, int* ctarget):
    """ map hashable lists of query and target into lists of integers"""
    query_vals = set(query)
    target_vals = set(target)
    if additional_equalities is not None:
        equ1_vals = [a for a, b in additional_equalities]
        equ2_vals = [b for a, b in additional_equalities]
    else:
        equ1_vals = equ2_vals = []
    input_mapping = {
        symbol: idx
            for idx, symbol in enumerate(query_vals.union(target_vals, equ1_vals, equ2_vals))
    }
    
    max_idx = max(list(input_mapping.values()))

    for idx, c in enumerate(query):
        cquery[idx] = <int> input_mapping[c]
    for idx, c in enumerate(target):
        ctarget[idx] = <int> input_mapping[c]

    if additional_equalities is not None:
            additional_equalities = [
                (input_mapping[a], input_mapping[b])
                for a, b in additional_equalities
                if a in input_mapping and b in input_mapping]
    return additional_equalities

def align(query, target, mode="NW", task="distance", k=-1, additionalEqualities=None):
    """ Align query with target using edit distance.
    @param {str or bytes or iterable of hashable objects}
    @param {str or bytes or iterable of hashable objects}
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
            List of pairs of characters or hashable objects, where each pair defines two values as equal.
            This way you can extend edlib's definition of equality (which is that each symbol is equal only
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
    # Transform python sequences of hashables into an array of integers.
    cdef int query_len = len(query)
    cdef int target_len = len(target)
    cdef int* cquery = <int *> malloc(query_len * sizeof(int))
    cdef int* ctarget = <int *> malloc(target_len * sizeof(int))

    additionalEqualities = map_to_int_array(query,target, additionalEqualities, query_len,target_len, cquery, ctarget)
    # Build an edlib config object based on given parameters.
    cdef cedlib.EdlibAlignConfig[int] cconfig = cedlib.edlibDefaultAlignConfig[int]()

    if k is not None: cconfig.k = k

    if mode == 'NW': cconfig.mode = cedlib.EDLIB_MODE_NW
    if mode == 'HW': cconfig.mode = cedlib.EDLIB_MODE_HW
    if mode == 'SHW': cconfig.mode = cedlib.EDLIB_MODE_SHW

    if task == 'distance': cconfig.task = cedlib.EDLIB_TASK_DISTANCE
    if task == 'locations': cconfig.task = cedlib.EDLIB_TASK_LOC
    if task == 'path': cconfig.task = cedlib.EDLIB_TASK_PATH

    if additionalEqualities is None:
        cconfig.additionalEqualities = NULL
        cconfig.additionalEqualitiesLength = 0
    else:
        cconfig.additionalEqualities = <cedlib.EdlibEqualityPair[int]*> PyMem_Malloc(len(additionalEqualities)
                                                                          * cython.sizeof(cedlib.EdlibEqualityPair[int]))
        for i in range(len(additionalEqualities)):
            cconfig.additionalEqualities[i].first = <int> additionalEqualities[i][0]
            cconfig.additionalEqualities[i].second = <int> additionalEqualities[i][1]
        cconfig.additionalEqualitiesLength = len(additionalEqualities)

    # Run alignment -- need to get len before disabling the GIL
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


def getNiceAlignment(alignResult, query, target, gapSymbol="-"):
    """ Output alignments from align() in NICE format
    @param {dictionary} alignResult, output of the method align() 
        NOTE: The method align() requires the argument task="path"
    @param {string} query, the exact query used for alignResult
    @param {string} target, the exact target used for alignResult
    @param {string} gapSymbol, default "-"
        String used to represent gaps in the alignment between query and target
    @return Alignment in NICE format, which is human-readable visual representation of how the query and target align to each other. 
        e.g., for "telephone" and "elephant", it would look like:
           telephone
            |||||.|.
           -elephant
        It is represented as dictionary with following fields:
          - {string} query_aligned
          - {string} matched_aligned ('|' for match, '.' for mismatch, ' ' for insertion/deletion)
          - {string} target_aligned
        Normally you will want to print these three in order above joined with newline character.
    """
    if type(alignResult) is not dict:
        raise Exception("The object alignResult is expected to be a python dictionary. Please check the input alignResult.")

    if 'locations' not in alignResult.keys():
        raise Exception("The object alignResult is expected to contain a field 'locations'. Please check the input alignResult.")

    target_pos = alignResult["locations"][0][0]
    if target_pos == None:
        target_pos = 0
    query_pos = 0 ## 0-indexed
    target_aln = []
    match_aln = []
    query_aln = []

    if 'cigar' not in alignResult.keys():
        raise Exception("The object alignResult is expected to contain a CIGAR string. Please check the input alignResult.")

    cigar = alignResult["cigar"]

    if alignResult["cigar"] == '' or alignResult["cigar"] == None:
        raise Exception("The object alignResult contains an empty CIGAR string. Users must run align() with task='path'. Please check the input alignResult.")        

    ###
    ## cigar parsing, motivated by yech1990: https://github.com/Martinsos/edlib/issues/127
    ##
    ## 'num_occurences' == Number of occurrences of the alignment operation
    ## 'alignment_operation' == Cigar symbol/code that represent an alignment operation
    ## 
    for num_occurrences, alignment_operation in re.findall("(\d+)(\D)", cigar):
        num_occurrences = int(num_occurrences)
        if alignment_operation == "=":
            target_aln += target[target_pos : target_pos + num_occurrences]
            target_pos += num_occurrences
            query_aln += query[query_pos : query_pos + num_occurrences]
            query_pos += num_occurrences
            match_aln += ["|"] * num_occurrences
        elif alignment_operation == "X":
            target_aln += target[target_pos : target_pos + num_occurrences]
            target_pos += num_occurrences
            query_aln += query[query_pos : query_pos + num_occurrences]
            query_pos += num_occurrences
            match_aln += ["."] * num_occurrences
        elif alignment_operation == "D":
            target_aln += target[target_pos : target_pos + num_occurrences]
            target_pos += num_occurrences
            query_aln += [gapSymbol] * num_occurrences
            query_pos += 0
            match_aln += [gapSymbol] * num_occurrences
        elif alignment_operation == "I":
            target_aln += [gapSymbol] * num_occurrences
            target_pos += 0
            query_aln += query[query_pos : query_pos + num_occurrences]
            query_pos += num_occurrences
            match_aln += [gapSymbol] * num_occurrences
        else:
            raise Exception("The CIGAR string from alignResult contains a symbol not '=', 'X', 'D', 'I'. Please check the validity of alignResult and alignResult.cigar")
    if isinstance(query, str) and isinstance(target, str):
        query_aln = "".join(query_aln)
        match_aln = "".join(match_aln)
        target_aln = "".join(target_aln) 
    alignments = {
        'query_aligned': query_aln,
        'matched_aligned': match_aln,
        'target_aligned': target_aln
    }

    return alignments


