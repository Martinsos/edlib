#!/usr/bin/env bash

# Very similar or equal proteins
../src/aligner -a NW \
    ../test_data/artificial_data/randProt100000_1.fasta \
    ../test_data/artificial_data/randProt100000_1_mutated.fasta

../src/aligner -a NW \
    ../test_data/artificial_data/randProt1000000.fasta \
    ../test_data/artificial_data/randProt1000000.fasta
                     
# Very different proteins
../src/aligner -a NW \
    ../test_data/artificial_data/randProt100000_1.fasta \
    ../test_data/artificial_data/randProt100000_2.fasta



# Alignment
../src/aligner -a HW -k 500 -c -s \
    ../test_data/genome_data/e_coli_DH1_illumina_1x1000.fasta \
    ../test_data/genome_data/e_coli_536.fasta



# Mapping genome data
../src/aligner -a HW -s \
    ../test_data/genome_data/e_coli_DH1_illumina_10x1000.fasta \
    ../test_data/genome_data/e_coli_536.fasta

