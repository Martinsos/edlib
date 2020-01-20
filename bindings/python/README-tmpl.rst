..  [[[cog
        
    import cog
    import edlib
       
    def indent(text, indentation = "    "):
        return indentation + text.replace("\n", "\n" + indentation)

    def comment(text):
        return "# " + text.replace("\n", "\n# ")

    def cogOutExpression(expr):
        cog.outl(indent(expr))
        cog.outl(indent(comment(str(eval(expr)))))

    ]]]
    [[[end]]]

=====
Edlib
=====

Lightweight, super fast library for sequence alignment using edit (Levenshtein) distance.

Popular use cases: aligning DNA sequences, calculating word/text similarity.

..  [[[cog
   
    cog.outl()
    cog.outl(".. code:: python")
    cog.outl()
       
    cogOutExpression('edlib.align("elephant", "telephone")')
    cog.outl()
       
    cog.outl(indent(comment('Works with unicode characters (or any other iterable of hashable objects)!')))
    cogOutExpression('edlib.align("ты милая", "ты гений")')
    cog.outl()

    cogOutExpression('edlib.align("AACG", "TCAACCTG", mode = "HW", task = "path")')
    cog.outl()

    cog.outl(indent('query = "elephant"; target = "telephone"'))
    cog.outl(indent(comment('NOTE: `task` has to be "path" in order to get nice alignment.')))
    cog.outl(indent('result = edlib.align(query, target, task = "path")'))
    cog.outl(indent('nice = edlib.getNiceAlignment(result, query, target)'))
    cog.outl(indent('print("\\n".join(nice.values()))'))

    query = "elephant"; target = "telephone"
    result = edlib.align(query, target, task = "path")
    nice = edlib.getNiceAlignment(result, query, target)
    output = "\n".join(nice.values())
    cog.outl(indent(comment(output)))
    cog.outl()

    ]]]

.. code::
   
   {{ Basic code examples will be generated here. }}

..  [[[end]]]

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

..  [[[cog

    import pydoc

    help_str = pydoc.plain(pydoc.render_doc(edlib.align, "%s"))

    cog.outl()
    cog.outl('Output of ``help(edlib.align)``:')
    cog.outl()
    cog.outl('.. code::\n')
    cog.outl(indent(help_str))

    ]]]

.. code::
   
   {{ Content of help(edlib.align) will be generated here. }}
    
..  [[[end]]]

getNiceAlignment()
------------------

.. code:: python

    getNiceAlignment(alignResult, query, target)

Represents alignment from ``align()`` in a visually attractive format.

..  [[[cog

    import pydoc

    help_str = pydoc.plain(pydoc.render_doc(edlib.getNiceAlignment, "%s"))

    cog.outl()
    cog.outl('Output of ``help(edlib.getNiceAlignment)``:')
    cog.outl()
    cog.outl('.. code::\n')
    cog.outl(indent(help_str))

    ]]]

.. code::

   {{ Content of help(edlib.getNiceAlignment) will be generated here. }}

..  [[[end]]]


-----
Usage
-----

..  [[[cog

    cog.outl()
    cog.outl(".. code:: python")
    cog.outl()

    cog.outl(indent('import edlib'))
    cog.outl()

    cogOutExpression('edlib.align("ACTG", "CACTRT", mode="HW", task="path")')
    cog.outl()

    cog.outl(indent(comment('You can provide additional equalities.')))
    cogOutExpression('edlib.align("ACTG", "CACTRT", mode="HW", task="path", additionalEqualities=[("R", "A"), ("R", "G")])')
    cog.outl()

    ]]]

.. code::

   {{ Additional usage examples will be generated here. }}

..  [[[end]]]
   

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

Check out `Edlib python package on Github <https://github.com/Martinsos/edlib/tree/master/bindings/python>`_.

