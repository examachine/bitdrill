# Bitdrill

_Author: Eray Ozkural, PhD_

## Description

Bitdrill is a highly efficient sequential frequent itemset mining tool, library, and assorted scripts, tools and bits and pieces.

The algorithm uses kDCI up to level 2, and then uses a vertical mining algorithm which dynamically switches between tidlists or a dense representation for items according to sparsity.

In our benchmarks, bitdrill turned out to be the *fastest* frequent itemset mining algorithm.

The package also includes other sequential miners for comparison and inclusion in a larger framework that permits some algorithmic experimentation. In particular, take a look at the python scripts which might be quite useful when prototyping an algorithm.

## Package contents

* aux: auxiliary data mining tools
* bin: sh scripts and local binary install dir
* build: justmake build configuration
* config: automatic configuration files
* data: test and benchmark data
  * assoc-gen: synthetic data generator
  * graphs: some sample F2 graphs
  * test: small test datasets
* doc: documents (see README's)
* justmake: justmake Makefile library
* lib: local library install dir
* perf: python benchmark scripts
* scripts: python data mining scripts
* src: C++, and haskell source code

KDCI, AIM, DCIClosed codes have been used for some of the programs in the distribution.

## Citation

Bitdrill is the sequential miner used in this paper:
http://ieeexplore.ieee.org/document/5703072/

Eray Özkural, Bora Uçar, Cevdet Aykanat:
Parallel Frequent Item Set Mining with Selective Item Replication. IEEE Trans. Parallel Distrib. Syst. 22(10): 1632-1640 (2011)

Please cite it if you use it in your research.

We are making this tool publicly available for the sake of sharing our results with the data mining community. Thanks a lot to the researchers in the data mining and parallel computing community who read our work and cited the paper. We are likely going to release the parallel version, as well, but it needs a bit of a cleanup.
