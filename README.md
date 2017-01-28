# Bitdrill

Bitdrill is a high-performance sequential frequent itemset mining tool, library, and assorted scripts, tools and bits and pieces.

The algorithm uses kDCI up to level 2, and then uses a vertical mining algorithm which dynamically switches between tidlists or a dense representation for items according to sparsity.

The package also includes other sequential miners for comparison and inclusion in a larger framework that permits some algorithmic experimentation. In particular, take a look at the python scripts which might be quite useful when prototyping an algorithm.

Bitdrill is the sequential miner used in this paper:
http://ieeexplore.ieee.org/document/5703072/

Eray Özkural, Bora Uçar, Cevdet Aykanat:
Parallel Frequent Item Set Mining with Selective Item Replication. IEEE Trans. Parallel Distrib. Syst. 22(10): 1632-1640 (2011)

Please cite it if you use it in your research.

We are making this tool publicly available for the sake of sharing our results with the data mining community. Thanks a lot to the researchers in the data mining and parallel computing community who read our work and cited the paper. We are likely going to release the parallel version, as well, but it needs a bit of a cleanup.

Just using make should build everything, but to avoid the compilation of auxiliary tools and everything you might want to try:
$ make programs

Eray Ozkural, PhD

