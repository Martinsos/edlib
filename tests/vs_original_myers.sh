#!/usr/bin/env bash

# Comparison of my code with original myers code from 1998.

ORIG_MYERS_DIR="/home/martin/Dropbox/Mile/SIMD_align/Myers/myers"
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

TIMEFORMAT=%R  # time will print real time

# @param  Name of query file
# @param  Name of target file
# @param  List of values to be used for k
function run_tests {
    for K in $3; do
        echo -e "\nk: $K"
        echo -e "Original Myers:"
        cd $ORIG_MYERS_DIR   
        time ./myers `cat $1` $K $2 > /dev/null

        echo -e "My Myers:"
        cd $DIR
        time ../src/aligner -a HW -k $K -s \
                 ../test_data/genome_data/$1 \
                 ../test_data/genome_data/$2 > /dev/null
    done
}


echo -e "\n############## Query of length 50 ##############"
# Best score is 0 for this dataset
run_tests e_coli_DH1_illumina_1x50.fasta e_coli_536.fasta "0 5 10 25 50"

echo -e "\n############## Query of length 1000 ##############"
# Best score is 30 for this dataset
run_tests e_coli_DH1_illumina_1x1000.fasta e_coli_536.fasta "0 10 50 100 500"

echo -e "\n############## Query of length 10000 ##############"
# Best score is 4911 for this dataset
run_tests e_coli_DH1_illumina_1x10000.fasta e_coli_536.fasta "0 50 500 1000 5000"
