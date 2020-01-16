=====
Edlib
=====

Lightweight, super fast library for sequence alignment using edit (Levenshtein) distance.

Popular use cases: aligning DNA sequences, calculating word/text similarity.

.. code:: python

    edlib.align("elephant", "telephone")
    # {'editDistance': 3, 'alphabetLength': 8, 'locations': [(None, 8)], 'cigar': None}

    # Works with unicode characters (or any other iterable of hashable objects)!
    edlib.align("ты милая", "ты гений")
    # {'editDistance': 5, 'alphabetLength': 12, 'locations': [(None, 7)], 'cigar': None}

    query = "AACG"; target = "TCAACCTG" 
    result = edlib.align(query, target, mode = "HW", task = "path")
    # {'editDistance': 1, 'alphabetLength': 4, 'locations': [(2, 4), (2, 5)], 'cigar': '3=1I'}

    query = "elephant"; target = "telephone"
    result = edlib.align(query, target, task = "path")
    nice = edlib.getNiceAlignment(result, query, target)
    print("\n".join(nice.values()))
    # -elephant
    # -|||||.|.
    # telephone

Edlib is actually a C/C++ library, and this package is it's wrapper for Python.
Python Edlib has mostly the same API as C/C++ Edlib, so feel free to check out `C/C++ Edlib docs <http://github.com/Martinsos/edlib>`_ for more code examples, details on API and how Edlib works.

--------
Features
--------

* Calculates **edit distance**.
* It can find **optimal alignment path** (instructions how to transform first sequence into the second sequence).
* It can find just the **start and/or end locations of alignment path** - can be useful when speed is more important than having exact alignment path.
* Supports **multiple alignment methods**: global(**NW**), prefix(**SHW**) and infix(**HW**), each of them useful for different scenarios.
* You can **extend character equality definition**, enabling you to e.g. have wildcard characters, to have case insensitive alignment or to work with degenerate nucleotides.
* It can easily handle small or **very large** sequences, even when finding alignment path.
* **Super fast** thanks to Myers's bit-vector algorithm.

**NOTE**: **Alphabet length has to be <= 256** (meaning that query and target together must have <= 256 unique values).

------------
Installation
------------
::

    pip install edlib

---
API
---

Edlib has two functions, ``align()`` and ``getNiceAlignment()``:

align()
-------

.. code:: python

    align(query, target, [mode], [task], [k], [additionalEqualities])

Aligns ``query`` against ``target`` with edit distance.

``query`` and ``target`` can be strings, bytes, or any iterables of hashable objects, as long as all together they don't have more than 256 unique values.

..
   [[[cog
       import cog
       import pydoc
       import edlib

       help_str = pydoc.render_doc(edlib.align, "%s", renderer=pydoc.plaintext)
       indentation = '    '

       cog.outl("")
       cog.outl("Output of ``help(edlib.align)``:")
       cog.outl("")
       cog.outl(".. code::\n")
       cog.outl(indentation + help_str.replace('\n', '\n' + indentation))
.. ]]]

Run ``help(edlib.align)`` to learn more.
    
..  [[[end]]]

getNiceAlignment()
------------------

.. code:: python

    getNiceAlignment(alignResult, query, target)

Represents alignment from ``align()`` in a visually attractive format.

..
   [[[cog
       import cog
       import pydoc
       import edlib

       help_str = pydoc.render_doc(edlib.getNiceAlignment, "%s", renderer=pydoc.plaintext)
       indentation = '    '

       cog.outl("")
       cog.outl("Output of ``help(edlib.getNiceAlignment)``:")
       cog.outl("")
       cog.outl(".. code::\n")
       cog.outl(indentation + help_str.replace('\n', '\n' + indentation))
.. ]]]

Run ``help(edlib.getNiceAlignment)`` to learn more.
    
..  [[[end]]]


-----
Usage
-----
.. code:: python

    import edlib

    result = edlib.align("ACTG", "CACTRT", mode="HW", task="path", additionalEqualities=[("R", "A"), ("R", "G")])
    print(result["editDistance"])  # 0
    print(result["alphabetLength"])  # 5
    print(result["locations"])  # [(1, 4)]
    print(result["cigar"])  # "4="

    result = edlib.align("elephant", "telephone", task="path")  ## users must use 'task="path"' 
    niceAlign = edlib.getNiceAlignment(result, "elephant", "telephone")
    print(niceAlign['query_aligned'])  #   "-elephant"
    print(niceAlign['matched_aligned'])  # "-|||||.|."
    print(niceAlign['target_aligned'])  #  "telephone"




---------
Benchmark
---------

I run a simple benchmark on 7 Feb 2017 (using timeit, on Python3) to get a feeling of how Edlib compares to other Python libraries: `editdistance <https://pypi.python.org/pypi/editdistance>`_ and `python-Levenshtein <https://pypi.python.org/pypi/python-Levenshtein>`_.

As input data I used pairs of DNA sequences of different lengths, where each pair has about 90% similarity.

::

   #1: query length: 30, target length: 30
   edlib.align(query, target): 1.88µs
   editdistance.eval(query, target): 1.26µs
   Levenshtein.distance(query, target): 0.43µs

   #2: query length: 100, target length: 100
   edlib.align(query, target): 3.64µs
   editdistance.eval(query, target): 3.86µs
   Levenshtein.distance(query, target): 14.1µs

   #3: query length: 1000, target length: 1000
   edlib.align(query, target): 0.047ms
   editdistance.eval(query, target): 5.4ms
   Levenshtein.distance(query, target): 1.9ms

   #4: query length: 10000, target length: 10000
   edlib.align(query, target): 0.0021s
   editdistance.eval(query, target): 0.56s
   Levenshtein.distance(query, target): 0.2s

   #5: query length: 50000, target length: 50000
   edlib.align(query, target): 0.031s
   editdistance.eval(query, target): 13.8s
   Levenshtein.distance(query, target): 5.0s

----
More
----

Check out `C/C++ Edlib docs <http://github.com/Martinsos/edlib>`_ for more information about Edlib!

-----------
Development
-----------

Run :code:`make build` to generate an extension module as .so file. You can test it then by importing it from python interpreter :code:`import edlib` and running :code:`edlib.align(...)` (you have to be positioned in the directory where .so was built). This is useful for testing while developing.

Run :code:`make sdist` to create a source distribution, but not publish it - it is a tarball in dist/ that will be uploaded to pip on `publish`. Use this to check that tarball is well structured and contains all needed files, before you publish.
Good way to test it is to run :code:`sudo pip install dist/edlib-*.tar.gz`, which will try to install edlib from it, same way as pip will do it when it is published.

Run :code:`make publish` to create a source distribution and publish it to the PyPI. Use this to publish new version of package.
Make sure to bump the version in `setup.py` before publishing, if needed.

:code:`make clean` removes all generated files.
