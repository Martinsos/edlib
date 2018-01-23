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
// ]

Channel.fromPath(params.inputDataDir + '/Enterobacteria_Phage_1/mutated_*_perc.fasta')
  .combine([[file(params.inputDataDir + '/Enterobacteria_Phage_1/Enterobacteria_phage_1.fasta'), 'NW', -1]])
  .combine([0, 1])  // Both with path and without.
  .tap{enterobacteriaEdlib}.tap{enterobacteriaSeqan}
  .filter{it[4] == 0}.tap{enterobacteriaParasail}  // Parasail can't find path, so skip that.

Channel.fromPath(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/*.fasta')
  .combine([[file(params.inputDataDir + '/E_coli_DH1/e_coli_DH1.fasta'), 'HW', -1]])
  .combine([0, 1])  // Both with path and without.
  .tap{eColiInfixEdlib}
  .filter{it[4] == 0}.tap{eColiInfixSeqan}  // Seqan takes too much memory to find path, so skip.

Channel.from([[file(params.inputDataDir + '/E_coli_DH1/mason_illumina_read_10kbp/e_coli_DH1_illumina_1x10000.fasta'),
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
  .tap{eColiInfixFixedKEdlib}.tap{eColiInfixFixedKMyers}

edlibTasks = enterobacteriaEdlib.mix(eColiInfixEdlib, eColiInfixFixedKEdlib)
seqanTasks = enterobacteriaSeqan.mix(eColiInfixSeqan)
parasailTasks = enterobacteriaParasail
myersTasks = eColiInfixFixedKMyers
//-----------------------------------------//

// TODO: Maybe have just one process, take aligner as an extra parameter, and then
//       have IF clauses to choose aligner?

// process edlib {
//   input:
//   set file(query), file(target), mode, k, path from edlibTasks
//
//   output:
//   set file(query), file(target), mode, k, path, stdout into edlibResults
//
//   shell:
//   // TODO: Now I print CIG_STD because there is no other way to get score. If I had something like
//   // -f NONE that still prints score I could avoid printing alignment, which can be very big.
//   '''
//   if [ !{path} = 0 ]; then
//       output=$(edlib-aligner -m !{mode} -k !{k} !{query} !{target})
//       score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
//   else
//       output=$(edlib-aligner -m !{mode} -p -f CIG_STD -k !{k} !{query} !{target})
//       score=$(echo "$output" | grep "score =" | cut -d "=" -f2)
//   fi
//   time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
//   echo $time $score
//   '''
// }
//
// edlibResults.subscribe {
//   println "edlib: $it"
// }
//
// process parasail {
//   input:
//   set file(query), file(target), mode, k, path from parasailTasks
//
//   output:
//   set file(query), file(target), mode, k, path, stdout into parasailResults
//
//   when:
//   path == 0 && mode == 'NW'  // Parasail 1.1 can not find alignment path and supports only 'NW' mode.
//
//   shell:
//   '''
//   output=$(parasail_aligner -t 1 -d -e 1 -o 1 -M 0 -X 1 -a nw_striped_32 -f !{target} -q !{query})
//   time=$(echo "$output" | grep "alignment time" | cut -d ":" -f2 | cut -d " " -f2)
//   score=$(($(head -n 1 parasail.csv | cut -d "," -f5) * -1))
//   rm parasail.csv
//   echo $time $score
//   '''
// }
//
// parasailResults.subscribe {
//   println "parasail: $it"
// }
//
// process seqan {
//   input:
//   set file(query), file(target), mode, k, path from seqanTasks
//
//   output:
//   set file(query), file(target), mode, k, path, stdout into seqanResults
//
//   shell:
//   '''
//   if [ !{path} = 0 ]; then
//       output=$(seqan-aligner -m !{mode} -t !{query} !{target})
//   else
//       output=$(seqan-aligner -m !{mode} -t -p -s !{query} !{target})
//   fi
//   time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
//   score=$(echo "$output" | grep "Seqan Score:" | cut -d " " -f4)
//   score=$(($score * -1))
//   echo $time $score
//   '''
// }
//
// seqanResults.subscribe {
//   println "seqan: $it"
// }

process myers {
  input:
  set file(query), file(target), mode, k, path from myersTasks

  output:
  set file(query), file(target), mode, k, path, stdout into myersResults

  when:
  path == 0 && mode == 'HW'  // Myers can not find alignment path and supports only 'HW' mode.

  shell:
  '''
  tail -n +2 !{query} | tr -d '\n' > queryMyers.fasta
  tail -n +2 !{target} | tr -d '\n' > targetMyers.fasta
  output=$({ time -p myers $(cat queryMyers.fasta) !{k} targetMyers.fasta; } 2>&1)
  rm queryMyers.fasta targetMyers.fasta
  echo $output
  time=$(echo "$output" | grep "real" | cut -d " " -f2)
  echo $time
  '''
}

myersResults.subscribe {
  println "myers: $it"
}
