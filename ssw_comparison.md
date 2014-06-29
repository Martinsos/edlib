# Speed comparison of Myers against SSW library.
All times are in seconds. Tests were performed on Intel Core i3 M 350 @ 2.27GHz with 4GB RAM.

## Read alignment test
I generated 10 reads of length 1000 from E.Coli genome (e_coli_DH1.fasta).  
I tested aligners by aligning reads to e_coli_536.fasta.
I obtained E.Coli genomes from ncbi base.
To generate N reads of length n from e_coli_DH1 I used tool `mason`:
  `./mason illumina -N <N> -n <n> -o reads.fasta e_coli_DH1.fasta`

In following tests, I aligned reads from e\_coli\_DH1 to e\_coli\_536.

I executed aligners using following commands:
* EDLIB: `./aligner -a HW reads.fasta e_coli_536.fasta`
* SSW: `./ssw_test e_coli_536.fasta reads.fasta`

Following table shows performance of EDLIB and SSW for different lengths of reads.

|       | N = 10, n = 1000 | N = 100, n = 100 | N = 200, n = 50 |
| ----- | ---------------- | ---------------- | --------------- |
| EDLIB |       12.3       |      12.8        |       16.8      |
|  SSW  |       21.0       |      49.0        |       80.0      |

## Protein comparison test
### Similar proteins
I generated two very similar proteins, both of length ~ 100000, and compared them with both aligners.  
I used NW mode for Myers.  
Myers: `./aligner -a NW randProt100000_1.fasta randProt100000_1_mutated.fasta`  
SSW: `./ssw_test -p randProt100000_1.fasta randProt100000_1_mutated.fasta`

Time:  
Myers(NW): 0.01s  
SSW: 7.00s

### Different proteins
I generated two very different proteins, both of length ~ 100000, and compared them with both aligners.  
I used NW mode for Myers.  
Myers: `./aligner -a NW randProt100000_1.fasta randProt100000_2.fasta`  
SSW: `./ssw_test -p randProt100000_1.fasta randProt100000_2.fasta`

Time:  
Myers(NW): 1.56s
SSW: 7.00s
