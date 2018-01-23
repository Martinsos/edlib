#!/usr/bin/env nextflow

params.numTests = 1  // TODO: actually use this. Generate duplicate tasks, and then average them later.
params.inputDataDir = '../../test_data'

//------- Experiments to conduct. --------//
// Experiment is of shape: [
//   file(query)
//   file(target)
//   mode - 'HW', 'NW' or 'SHW'.
//   k - max boundary for edit distance.
//   path - 0 if we are looking only for edit distance, 1 if we are also looking for alignment path.
//   aligner - name of aligner to execute this experiment with.
//             Could be 'edlib', 'seqan', 'parasail' or 'myers'.
// ]

// enterobacteriaExperiments = Channel.fromPath(params.inputDataDir + '/Enterobacteria_Phage_1/mutated_*_perc.fasta')
//   .combine([[file(params.inputDataDir + '/Enterobacteria_Phage_1/Enterobacteria_phage_1.fasta'), 'NW', -1]])
//   .combine([0, 1])  // Both with path and without.
//   .combine(['edlib', 'seqan', 'parasail'])

// eColiExperiments = Channel.fromPath(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/*.fasta')
//   .combine([[file(params.inputDataDir + '/E_coli_DH1/e_coli_DH1.fasta'), 'HW', -1]])
//   .combine([0, 1])  // Both with path and without.
//   .combine(['edlib', 'seqan', 'parasail'])
//   .filter{!(it[4] == 1 && it[5] == 'seqan')}  // Seqan takes too much memory to find path, so skip.
//
fixedKExperiments = Channel.from([[file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/e_coli_DH1_illumina_1x10000.fasta'),
                                   file(params.inputDataDir + '/E_coli_DH1/e_coli_DH1.fasta'), 'HW', 100, 0]])
  .mix(
    Channel.from(file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/e_coli_DH1_illumina_1x10000.fasta'),
                 file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/mutated_97_perc.fasta'),
                 file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/mutated_94_perc.fasta'),
                 file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/mutated_90_perc.fasta'))
      .combine([[file(params.inputDataDir + '/E_coli_DH1/e_coli_DH1.fasta'), 'HW', 1000, 0]])
    // TODO: for some reason Myers fails for this one, figure out why.
    // ,Channel.fromPath(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/*')
    //   .combine([[file(params.inputDataDir + '/E_coli_DH1/e_coli_DH1.fasta'), 'HW', 10000, 0]])
    )
  .combine(['edlib', 'myers'])

experiments = //Channel.create().mix(
//  enterobacteriaExperiments,
//  eColiExperiments,
  fixedKExperiments
//)
//-----------------------------------------//


process align {
  input:
  set file(query), file(target), mode, k, path, aligner from experiments

  output:
  set file(query), file(target), mode, k, path, aligner, stdout into results

  when:
  // Here we define which epxeriments are ok for which aligner, in case there is tricky experiment.
  ((aligner == 'edlib')
   || (aligner == 'seqan')
   || (aligner == 'parasail' && path == 0 && mode == 'NW')
   || (aligner == 'myers' && path == 0 && mode == 'HW' && k >= 0))

  shell:
  if (aligner == 'edlib')
    // TODO: Now I print CIG_STD because there is no other way to get score. If I had something like
    // -f NONE that still prints score I could avoid printing alignment, which can be very big.
    '''
    if [ !{path} = 0 ]; then
        output=$(edlib-aligner -m !{mode} -k !{k} !{query} !{target})
        score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
    else
        output=$(edlib-aligner -m !{mode} -p -f CIG_STD -k !{k} !{query} !{target})
        score=$(echo "$output" | grep "score =" | cut -d "=" -f2)
    fi
    time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
    echo $time $score
    '''
  else if (aligner == 'seqan')
    '''
    if [ !{path} = 0 ]; then
        output=$(seqan-aligner -m !{mode} -t !{query} !{target})
    else
        output=$(seqan-aligner -m !{mode} -t -p -s !{query} !{target})
    fi
    time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
    score=$(echo "$output" | grep "Seqan Score:" | cut -d " " -f4)
    score=$(($score * -1))
    echo $time $score
    '''
  else if (aligner == 'parasail')
    '''
    output=$(parasail_aligner -t 1 -d -e 1 -o 1 -M 0 -X 1 -a nw_striped_32 -f !{target} -q !{query})
    time=$(echo "$output" | grep "alignment time" | cut -d ":" -f2 | cut -d " " -f2)
    score=$(($(head -n 1 parasail.csv | cut -d "," -f5) * -1))
    rm parasail.csv
    echo $time $score
    '''
  else if (aligner == 'myers')
    '''
    tail -n +2 !{query} | tr -d '\n' > queryMyers.fasta
    tail -n +2 !{target} | tr -d '\n' > targetMyers.fasta
    output=$({ time -p myers $(cat queryMyers.fasta) !{k} targetMyers.fasta; } 2>&1)
    rm queryMyers.fasta targetMyers.fasta
    time=$(echo "$output" | grep "real" | cut -d " " -f2)
    echo $time ?  # TODO: Find score!
    '''
  else
    error "Invalid aligner: ${aligner}."
}

results
  .map{ [it[0].baseName, it[1].baseName, it[2], it[3], it[4], it[5], *(it[6].split(" "))] }
  .map{ [it.take(5).join('-'), *it] }
  .groupTuple()
  .map{ it.drop(1) }
  .view()
