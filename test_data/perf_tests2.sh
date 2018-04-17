#!/usr/bin/env bash

# Runs performance tests - compares Edlib with other aligners (Parasail, Seqan, Myers).

EDLIB=~/git/edlib/build/bin/edlib-aligner

TEST_DATA=.

########################## TEST RUNNERS ########################

function edlib {
    mode=$1
    query=$2
    target=$3
    num_tests=$4
    k=$5

    time_sum=0
    for i in $(seq $num_tests); do
        sleep 1
        output=$($EDLIB -m $mode -k $k $query $target)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
        time_sum=$(python -c "print($time_sum + $time)")
        echo ">" "#"$i $score $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo "Edlib:" $avg_time $score
}

function edlib_path {
    mode=$1
    query=$2
    target=$3
    num_tests=$4

    time_sum=0
    for i in $(seq $num_tests); do
        sleep 1
        output=$($EDLIB -m $mode -p -s $query $target)
        time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
        time_sum=$(python -c "print($time_sum + $time)")
        echo ">" "#"$i $time
    done
    avg_time=$(python -c "print($time_sum / $num_tests)")
    echo "Edlib (path):" $avg_time
}


############################ TESTS #############################


#E coli and its illumina read, HW. Only short reads (<= 500bp).
echo -e "\nE coli and its illumina read, HW"
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query in $(ls $TEST_DATA/E_coli_DH1/mason_illumina_reads/*0bp/*.fasta); do
    echo $query

    edlib HW $query $target 3 -1
    edlib_path HW $query $target 3
done

#E coli and its illumina read, HW. Only big reads (>= 1kbp).
echo -e "\nE coli and its illumina read, HW"
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query in $(ls $TEST_DATA/E_coli_DH1/mason_illumina_reads/*kbp/*.fasta); do
    echo $query

    edlib HW $query $target 3 -1
    edlib_path HW $query $target 3
done

#E coli and its prefix, SHW
echo -e "\nE coli and its prefix, SHW"
target=$TEST_DATA/E_coli_DH1/e_coli_DH1.fasta
for query in $(ls $TEST_DATA/E_coli_DH1/prefixes/*/mutated_*_perc.fasta); do
    echo $query

    edlib SHW $query $target 3 -1
    edlib_path SHW $query $target 3
done
