#!/usr/bin/env nextflow

params.numTests = 3
params.inputDataDir = '../test_data/'

runEdlibChannel = Channel.create()
runSeqanChannel = Channel.create()
runParasailChannel = Channel.create()

enterobacteriaTasksChannel = Channel.fromPath(params.inputDataDir + 'Enterobacteria_Phage_1/mutated_*_perc.fasta')
  .combine([[file(params.inputDataDir + 'Enterobacteria_Phage_1/Enterobacteria_phage_1.fasta'), 'NW', -1]])
  .combine([0, 1])  // Both with path and without.
// Make sure in parasail to skip tasks with path alignment!
enterobacteriaTasksChannel.into{runEdlibChannel; runSeqanChannel; runParasailChannel}


process edlib {
  input:
  // If path is 0, we look only for alignment score, otherwise we also look for alignment path.
  set file(query), file(target), mode, k, path from runEdlibChannel

  output:
  set file(query), file(target), mode, k, path, stdout into edlibResultChannel

  shell:
  // TODO: Can I also parallelize it on the number of tests? Now I do them one after another in this bash
  // script, but maybe I could have each of them as separate process and then Nextflow could also
  // parallelize that.
  // TODO: Now I print CIG_STD because there is no other way to get score. If I had something like
  // -f NONE that still prints score I could avoid printing alignment, which can be very big.
  '''
  time_sum=0
  for i in $(seq !{params.numTests}); do
      sleep 1
      if [ !{path} = 0 ]; then
          output=$(edlib-aligner -m !{mode} -k !{k} !{query} !{target})
          score=$(echo "$output" | grep "#0:" | cut -d " " -f2)
      else
          output=$(edlib-aligner -m !{mode} -p -f CIG_STD -k !{k} !{query} !{target})
          score=$(echo "$output" | grep "score =" | cut -d "=" -f2)
      fi
      time=$(echo "$output" | grep "Cpu time of searching" | cut -d " " -f5)
      time_sum=$(python -c "print($time_sum + $time)")
  done
  avg_time=$(python -c "print($time_sum / !{params.numTests})")
  echo $avg_time $score
  '''

}

edlibResultChannel.subscribe {
  println it
}
