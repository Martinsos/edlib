# Speed comparison of Myers against SSW library.

## Read alignment test
I generated 10 reads of length 1000 from E.Coli genome (e_coli_DH1.fasta).  
I tested aligners by aligning reads to e_coli_536.fasta.  
I obtained E.Coli genomes from ncbi base.  
To generate reads, I used tool `mason`:  
  `./mason illumina -N 10 -n 1000 -o e_coli_DH1_illumina_10x1000.fasta e_coli_DH1.fasta`

I run Myers in HW mode, and SSW with default options.  
I called them with following commands:  
  Myers: `./aligner -a HW e_coli_DH1_illumina_10x1000.fasta e_coli_536.fasta`  
  SSW: `./ssw_test e_coli_536.fasta e_coli_DH1_illumina_10x1000.fasta`

Time:  
Myers(HW): 17.10s  
SSW: 20.00s  

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
Myers(NW): 2.04s  
SSW: 7.00s
