I compared my code with Landau Vishkin that I extracted from SNAP.
Landau Vishkin from SNAP is using SHW mode.
I forked snap repository and changed apps/snap/Main.cpp so it acts like my aligner, but uses Landau Vishkin instead of Myers.

I tested basic Landau Vishkin, which finds just edit distance (not position and not alignment).

Command used for testing Landau Viskhin:
 ./snap -k K <queries.fasta> <target.fasta>

Command used for testing Myers:
 ./aligner -a SHW -k K <queries.fasta> <target.fasta> 

TEST 1: query: randProt100000_1.fasta, target: randProt100000_1_mutated.fasta, solution is 29
- Landau Vishkin: 0.000000s to 0.010000s (K = 64)
- Myers: 0.010000s to 0.020000s (K = 64)

TEST 2: query: randProt1000000.fasta, target: randProt1000000.fasta, solution is 0
- Landau Vishkin: 0.000000s (K = 64, K = 0)
- Myers: 0.1s (K = 64), 0.6 (K = 0)

Following tests are more suited for SHW because we actually have prefix here:
TEST 3: query: randProt1000000Prefix1000.fasta, target: randProt1000000.fasta, solution is 0
- LV: 0s (K <= 64)
- Myers: 0s (K <= 64)

TEST 4: query: randProt1000000Prefix1000_mutated.fasta, target: randProt1000000.fasta, solution is 105
- LV: 0s (K < 500)
- Myers: 0s (K < 500)

TEST 5: query: randProt1000000Prefix100000_mutated.fasta, target: randProt1000000.fasta, solution is 648
- LV: 0.01s (K = 1000)
- Myers: 0.03s (K = 1000)
