######################################################################
LCM: Linear time Closed itemset Minor
     Coded by Takeaki Uno,   e-mail:uno@nii.jp, 
        homepage:   http://research.nii.ac.jp/~uno/index.html
######################################################################

** This program is available for only academic use, basically.   **
** Anyone can modify this program, but he/she has to write down  **
** the change of the modification on the top of the source code. **
** Neither contact nor appointment to Takeaki Uno is needed.     **
** If one wants to re-distribute this code, do not forget to     **
** refer the newest code, and show the link to homepage          **
** of Takeaki Uno, to notify the news about LCM for the users.   **
** For the commercial use, please make a contact to Takeaki Uno. **

################################
####   Problem Definition   ####
################################

Let I be a set of items. An itemset is a subset of I. Let D be a transaction 
database such that each record (called transaction) is an itemset. Frequency
of an itemset is the number of transactions including the itemset. For a
given number t (called support), an itemset is said to be frequent
if its frequency is no less than t. A frequent itemset is called maximal
if it is included in no other frequent itemset, and called closed if it
is included in no other itemset of the same frequency. The task of
this program, LCM, is to enumerate (output, or count) all frequent
itemsets, all maximal frequent itemsets, or all frequent closed itemsets
in a given transaction database for given a support.


#####################
####    Usage    ####
#####################

    ==== How to Compile ====
Unzip the file into arbitrary directory, and execute "make".
Then you can see "lcm" (or lcm.exe) in the same directory.
In the case of ver. 2xx and 3, you see fim_all, fim_closed, and fim_maximal.

    ==== Command Line Options for ver.2xx and ver.3  ====
The parameter format follows the FIMI implementations.
(http://fimi.cs.helsinki.fi/) 
To enumerate frequent itemsets, execute fim_all. For frequent closed itemsets, 
execute fim_closed, and for maximal frequent itemsets, execute fim_maximal.
The first parameter is the input file name, the second is minimum support, 
and the third is the output file name. We can omit the third parameter, 
then no output file will be generated, and the program counts the number of 
solutions.

Example)
% fim_closed input-filename support [output-filename]

    ====  Command Line Options for ver.4, and ver.5  ====
To execute LCM, just type lcm and give some parameters as follows.

% lcm FCMfIq [options] input-filename support [output-filename]

"%" is not needed to type. It is a symbol to represent command line.
To see a simple explanation, just execute "lcm" without parameters.

"input-filename" is the filename of the input transaction database.
The 1st letter of input-filename must not be '-'. Otherwise it is 
regarded as an option. The input file format is written below. "support"
is the support, which is for given the threshold of frequency.
"output-filename" is the name of file to write the itemsets the program
finds. You can omit the output file to see only the number of frequent
itemsets in the database.

The first parameter is given to the program to indicate the task.
 F: enumerate frequent itemsets,
 C: enumerate closed frequent itemsets
 M: enumerate maximal frequent itemsets

For example, if you want to enumerate maximal frequent itemset, type 
"M" in the first parameter. Additionally, we can give the following
commands, to specify the output style: 

 f: output the frequency on the end of each itemset found,
 q: no output to standard output (including messages w.r.t. input data)
 I: output ID's of transactions including each itemset

The output format is explained below. The following options are to
restrict the itemset to be found. We can type them between the first 
parameter and the second parameter. Of course the option can be nothing.

 -l [num]: enumerate itemsets with size at least [num]  
 -u [num]: enumerate itemsets with size at most [num]  
 -U [num]: enumerate itemsets with frequency at most [num]
 -w [filename]: read weights of transactions from the file
 -K [num]: output the frequency of [num]-th largest frequent(closed/maximal)
          itemsets. (ver.5 only)

-l, -u, and -U specify the upper/lower bound for itemset/frequency.
itemsets exceed the upper/lower bound will be not output.

If we specify "-w" and filename, then LCM reads the file as weight 
database of transactions. Each i-th line of the file is regarded as
the weight of i-th transaction. With transaction weights, LCM finds
the itemsets such that the sum of weights of the transaction including
the itemset is no less than the support. Note that we can give only 
integers as weights. If you want to give real numbers, you can multiply
the weights so that they will be integers.
For version 5, we can give minus weights.

When we give -K option, LCM compute the frequency of the [num]-th largest
frequent(closed/maximal) itemsets, and output it to the standard output
(print it to the command line). By giving the output frequency to LCM, 
we can enumerate top-[num] frequent itemsets (closed, or maximal).


Examples)

- To find all frequent itemsets in "test.dat" for support 4, and size of
 no less than 3. Output frequencies of each itemsets found. Do not output 
 to file.

% lcm Ff -l 3 test.dat 4

- To find closed itemsets in "test.dat" with frequency at least 6, and
 sizes from 5 to 10. Output itemsets to "out.dat".
% lcm C -l 5 -u 10 test.dat 6 out.dat

- To find maximal frequent itemsets in "test.dat" with weight file
  "weight.dat" with frequency at least 8, output to "out.dat" with
   transaction IDs, and no output for standard output.
% lcm MqI -w weight.dat test.dat 8 out.dat


###############################
####   Input File Format   ####
###############################

Each line (row) of the input file is a transaction. The items included in 
a transaction is listed in a line. The items must be numbers begin from 0.
They can not be minus. The item numbers do not have to be continuous, but
notice that the program takes memory linear in the maximum item number.
The separator of numbers can be any non-numeric letter, such as 
","  " " ":" "a", etc.

Example)  ( "[EOF]" is the end of file )
0 1 2
1
2 3 4
4,1 2 3
2,1
[EOF]

#################################################################
####    Use General Names for Variables and Other Formats    ####
#################################################################

We can transform variable names in general strings to numbers so that we
can input the data to the program, by some script files. 

-- transnum.pl table-file [separator] < input-file > output-file
Read file from standard input, and give a unique number to each name written
by general strings (such as ABC, ttt), and transform every string name to 
a number, and output it to standard output. The mapping from string names to
numbers is output to table-file. The default character for the separator of 
the string names is " "(space). It can be changed by giving a character for
the option [separator]. For example, A,B is a string name, if the separator 
is space, but if we set the separator to ",", it is regarded as two names 
A and B. This is executed by "transnum.pl table-file "," < input-file...".

-- untransnum.pl table-file < input-file > output-file
According to the table-file output by transnum.pl, un-transform numbers to 
string names. The output of the program is composed of numbers, thus 
it is used if we want to transform to the original string names.
It reads file from standard output, and output to the standard output.

-- appendnum.pl
When we want to distinct the same words in different columns, use this 
script. This append column number to each words, so we can distinct them.
Then, by using transnum.pl, we transform the strings to numbers.

-- transpose.pl
Transpose the file. In the other words, consider the file as an adjacency
matrix, and output the transposed matrix, or exchange the positions of
items and transactions. For an input file, output the file in which 
the i-th line corresponds to item $i$, and includes the numbers j
such that i is included in the j-th line of the input file.

#########################################
####   Batch Files for Simple use    ####
#########################################

For general string names, we have several batch files scripts for basic usages
"exec_lcm", "exec_lcm_", "sep_lcm", or "sep_lcm_". For example, when a database
with "general item names" is, 

dog pig cat
cat mouse
cat mouse dog pig
cow horse
horse mouse dog
[EOF]

All these replace strings in the input database by numbers, execute LCM,
and replace the numbers in the output file by the original strings.
The usage of the scripts are

% exec_lcm [FCMfIq] input-filename support output-filename [options]

You have to specify F, C or M, and output filename. The separator of the
items is " " (blank, space). If you want to use other character as a
separator, use "sep_lcm". The usage is 

% sep_lcm separator [FCMfIq] input-filename support output-filename [options]

Almost same as "exec_lcm" but you have to specify separator at the fourth 
parameter. "exec_lcm_" and "sep_lcm_" are both for the aim to distinct 
the same items in the different columns. For example, it is used to the
database such that different items are there in different columns, but 
some special symbols, such as "- is for missing data", are used commonly.
An example is;

A true small
- true big
- false middle
B - -
C - middle
A true -
[EOF]

In the output file, the items are followed by "." and numbers where 
the numbers are the column number. For example, "dog.0" means the item
"dog" on the 0th(first) column.

The usage of them are the same as "exec_lcm" and "sep_lcm", respectively.
The scripts use files of the names "__tmp1__", "__tmp2__", and "__tmp3__", 
The file of these names will be deleted after the execution.

Example)

% exec_lcm F test2.dat 10 out.dat -w weight.dat -l 2

% sep_lcm_ "," C test3.dat 3 out.dat -U 5


#############################
####    Output Format    ####
#############################

When the program is executed, the program prints out the #items,
#transactions, and other features of the input database to standard
error. After the termination of the enumeration, it outputs the total
number of itemsets found (frequent/closed/maximal itemsets), and the 
numbers of itemsets of each size. For example, if there are 4 frequent
itemsets of size 1, 2 frequent itemsets of size 3, and 1 frequent itemset
of size 3, then the output to standard output will be,

9   <= total #frequent itemsets
1   <= #frequent itemsets of size 0 (empty set), it is always frequent
4   <= #frequent itemsets of size 1
3   <= #frequent itemsets of size 2
1   <= #frequent itemsets of size 3

If "q" is given in the first parameter, 

If output-filename was given, then the itemsets found are written to 
the output file. Each line of the output file is the list of items 
included in an itemset found, separated by " ". If "f" is given in 
the first parameter, the frequency follows each itemset, for example,

1 5 10 2 4 (22)

which means itemset {1,2,4,5,10} is included in 22 transactions.

The output itemsets are not sorted. If you want to sort it, use the script
"sortout.pl". The usage is just,

% sortout.pl < input-file > output-file

"input-file" is the name of file to which LCM output, and the sorted output
will be written in the file of the name "output-file".
The items of each itemset will be sorted in the increasing order of
items, and all the itemsets (lines) will be also sorted, by the
lexicographical order (considered as a string).
(Actually, you can specify separator like sortout.pl ",").



###########################
####    Performance    #### 
###########################

The performance of LCM is stable, for both computation time and memory use.
The initialization and preprocess time of LCM is linear in the size of 
input database. The computation time for finding frequent itemsets depends
on the number of itemsets found, and the minimum support.
When the minimum support is large and the number of the itemsets found is
small, the computation time for each itemset is relatively large. However, 
computation time per itemset decreases as the increase of the itemsets found,
and roughly speaking when the size of output file is equal to the input
database size, it will be constant, such as 1/1,000,000 sec (by PC with 
CPU of 2GHz.)

Memory usage of LCM is very stable. It is an advantage compared to other 
implementations. The memory usage of LCM is always linear in the size of
the input database. Approximately LCM uses integers at most three times
as much as the database size, which is the sum of the number of items of
each transaction. The memory usage of the other implementations increases 
as the increase of the number of frequent itemsets, but that of LCM does not.


######################################
####    Solving Other Problems    ####
######################################

- Enumerating Maximal Bipartite Cliques in a Bipartite Graph -

Enumerating all maximal bipartite cliques in a bipartite graph is
equivalent to enumerating all closed itemsets. LCM can be used for
this task. For a bipartite graph of two vertex sets A and B, construct
the database such that each line is the list of vertices incident to
a vertex in A. Then, by executing 

% lcm CI input-filename 1 output-filename

you can enumerate maximal bipartite cliques.


- Enumerating Maximal Bipartite Cliques in a Graph -

By transforming the graph, we can enumerate bipartite cliques in graphs.
For a given G=(V,E), we construct a graph G'=(V+V', E') where V' is a 
copy of V. If an edge (v,u) is in E, then, in E', vertex v of V and 
vertex u in V' are connected by an edge, and vertex u of V and vertex 
v in V' are connected by an edge. Then, G' is a bipartite graph, and 
a bipartite clique in G' is a bipartite clique in G, and vice versa.
Thus, giving G' to LCM, we can enumerate all maximal bipartite cliques
in general graphs.


- Enumerating Maximal Directed Bipartite Cliques in a directed Graph -

For a directed graph G=(V,A), a directed bipartite clique is vertex sets
B and C such that for any pair of a vertex b in B and c in C, there is 
an arc from b to c. In a similar way to the above, we construct a 
directed graph G'=(V+V', A') where V' is a copy of V. If an arc (v,u) is
in A, then, in A', there is an edge connecting vertex v of V and vertex u
in V' in A'. Then, G' is a bipartite graph, and a bipartite clique in G' 
is a directed bipartite clique in G, and vice versa. Thus, giving G' to LCM,
we can enumerate all maximal bipartite cliques in general graphs.


- Finding Pairs of Transactions Having Large Intersection -

Suppose that we want to see which pair of transactions having many common 
items, i.e., for a threshold t, we want to find all pairs of transactions
A and B such that at least t items are included in both A and B.
For the sake, execute lcm with -l 2 and -u 2, for example,

lcm F -l 2 -u 2 test.dat 20

then we can find all pairs of transactions having at least 20 common items.


- Mining "itemset" sets.
Consider a data base each whose record is a set of itemset (transaction
database). a set S of itemsets is included in a record R if and only if 
each itemset of S is included in some itemset in R. Note that an itemset
of R may include more than one itemsets of S. Here the problem is to enumerate
all sets of itemsets included in no less than theta records of the given
database. When we give a restriction of the size of itemsets in a pattern S
to be enumerated, say at most some small k, we can tract the problem by
using LCM. Transform each record of the database to an itemset by

 listing all the itemsets included in at least one itemsets in the record,
 and regarding each itemset listed as an item.

For example, an itemset {A,B,C} is considered as an item "{A,B,C}".
A frequent itemset in the database obtained corresponds to a frequent 
set of itemsets. They correspond to each other one-to-one. Moreover, 
by enumerating closed itemsets, we can ignore unnecessary patterns
automatically. If a frequent itemset includes a item {A,B,C}, corresponding
to an itemset {A,B,C} in the original problem, then without changing the
frequency we can add any item corresponding to a subset of {A,B,C} to the
itemset. A closed itemset always includes such subsets, thus a closed 
itemset is given by a set of itemsets such that any two itemsets does 
not satisfy the inclusion relation. Thus, we can discard unnecessary 
patterns automatically.


########################################################
####    Introductions to Frequent Itemset Mining    ####
########################################################

Let D be a database such that each record is transaction data. Here
transaction data is a set of items. Thus, the database is a set of
subsets of items. Such a database is called a "transaction database".

For example, let us see the following database of
transactions:

  transaction A: 1,2,5,6,7
  transaction B: 2,3,4,5
  transaction C: 1,2,7,8,9
  transaction D: 1,7,9
  transaction E: 2,7,9
  transaction E: 2,7,9
  transaction F: 2

Transaction A is a set of items 1, 2, 5, 6, and 7, and the
others are so on. Now we consider a problem of finding sets of items
which are included in many transactions, since such sets give some
interesting structures of the database. Since "many transactions" is
not well-defined, we introduce a number called "minimum support" or simply
"support", and consider that itemsets included in at least "minimum support"
transactions are included in many transactions. We call such itemsets 
"frequent itemsets", and the number of transactions including an itemset 
"frequency" of the itemset. For the above transaction database, by setting
minimum support to 3, frequent itemsets are

 {}, {1}, {2}, {7}, {9}, {1,7}, {1,9}, {2,7}, {2,9}, {7,9}, and {2,7,9}.
 
Note that the first itemset is the empty set, and it is considered as a 
frequent itemset. If the support is large, #frequent itemsets is small.
The number of frequent itemsets increases as the decrease of the support.

A popular usage of the frequent itemset mining is to find an interesting 
knowledge from database. In this sense, large support usually gives trivial
frequent itemset, which are not interesting. In the other hand. when we
set minimum support to a small number, the number of frequent itemsets 
will be so huge. Dealing with a large huge of itemsets is itself a hard 
task. Thus, next we consider how to decrease the number of itemsets to be
found, without missing interesting knowledge.

One approach to this task is to find only maximal frequent itemsets, 
which are included in no other frequent itemset. For example, the maximal
frequent itemsets in the above database are 

 {1,7}, {1,9}, and {2,7,9}.

The idea is that any frequent itemset is included in at least one maximal
frequent itemset, at least we can get the itemset from maximal frequent
itemsets. However, we can not know how much frequent they are.
For example, {2,7} can be obtained from {2,7,9}, but we can not know 
whether {2,7} is more frequent than {2,7,9} or not. In this sense we 
miss how much interesting each frequent itemset is.

Using closed itemsets, we can avoid this problem. An itemset is closed
if it is included in no other itemset of the same frequency. For example,
in the above database, the closed itemsets are

 {}, {2}, {2,5}, {7,9}, {1,7,9}, {2,7,9}, {1,2,7,9}, {2,3,4,5},
 {1,2,7,8,9}, and {1,2,5,6,7,9}.
 
Closed itemset is a maximal frequent itemset if the support is 
equal to its frequency. Thus, the set of closed itemsets is the collection
of maximal frequent itemsets for all possible supports. Any non-closed 
itemset is dominated by a closed itemset, thus we lose nothing about 
the frequency.


###################################################
####    Algorithms and Implementation Issue    #### 
###################################################

The basic idea of the algorithm is depth first search. Let D be a transaction
database, t be the minimum support, 1,...,n be items, T1,...,Tm be the
transactions in D. D(I) denotes the set of transactions including I. We denote
the largest item of an itemset I by tail(I). LCM first computes the frequency of
each itemset composed of one item. If an itemset {i} is frequent, then 
enumerate frequent itemsets obtained by adding one item to {i}. In this way,
recursively, LCM enumerates all frequent itemsets. To avoid duplications,
LCM adds items j to {i} only if j>i. This algorithm is written as follows:

  FrequentItemsetMining (D:database, I:itemset )
    Output I
    For each item j > tail(I), 
      if (I \cup j) is frequent, then FrequentItemsetMining (D, I\cup{j})

By calling FrequentItemsetMining (D, emptyset), we can enumerate 
all frequent itemsets. 

However, a straightforward implementation of this algorithm is very slow,
since computing frequency of (I\cup{j}) takes long time. To be fast, we use 
conditional database and occurrence deliver as follows.

   ==== Conditional database of itemset I ===
Conditional database of itemset I, denoted by D(I), is given by the
database obtained by

  1. D(I) := all transactions including I
  2. remove all unnecessary items from each transaction of D(I)
  3. merge the identical transactions into one.
   (do for all such identical transactions)

Here unnecessary item is an item satisfying 
  (a) included in less than t transactions of D(I), 
  (b) included in all transactions in D(I), or
  (c) less than the largest item in I.
Then, the frequency of itemset I plus J is the same, in D and D(I),
thus LCM uses D(I) instead of D in the recursive call with respect to I.
This technique is quite common in implementations of frequent itemset mining.

Constructing D(I) possibly takes much memory, when the recursion is deep.
Thus, LCM uses a slight modification.

  1. D(I) := IDs of all transactions including I
  2. If there are transactions T1,...,Tk become the same after deleting
   unnecessary items, make a new transaction equal to it, and replace the
   IDs of T1,...,Tk by the ID of new transaction

After terminating the recursive call with respect to I, we delete the 
transactions generated in the process. By this, we can bound the memory
usage for the transactions by twice the database size. From this, LCM
allocates the memory in the initialization, and never do again in the
main routine. Thus the memory usage is very stable, and is very fast
since computation time for memory allocation is not small in the
frequent itemset mining implementations.

   === occurrence deliver ===
Let T be a transaction database (subset family), a collection of subsets of
E = {1,...,n}, and T(S) be the set of transactions (subsets) in T including 
a subset S.
We suppose that each transaction is sorted. The occurrence deliver computes
T(i) for all i>j, for a given j, in time linear in the sum of their sizes.
First, for each i for which we compute T(i), we give an empty bucket. Then,
for each transaction t in T, we scan it in decreasing order of items until 
we meet the item less than j, and for each item i>j, insert t to the bucket
of i. After scanning all the transactions, the bucket of i is T(i).
When we have T(S) and want to compute T(S\cup {i}) for all i>tail(S), 
we can do in the same way by setting T:=T(S), and j:=tail(S). For the check
whether the current itemset is a maximal frequent itemset of not, we also
use the occurrence deliver. If there is no item i such that S\cup {i} is
frequent, then the frequent itemset S is maximal frequent itemset.

   === closed itemset mining ===
To enumerate closed itemsets, we use the ppc extension (prefix preserving
closure extension). For an itemset P, we define P(i) by the items of P which
are smaller than i. The closure C(P) of P is the closed itemset including P
and having the same frequency to P. When P is a closed itemset, we define
its core index, denoted by core_i(P), by the minimum item i such that
C(P(i)) = P. Then, a closed itemset P' is said to be a ppc extension of P
if and only if 

(i) P' = C(P\cup {i}) for some i>core_i(P).
(ii) P(i) = P'(i).

Note that P' has smaller frequency than P. It is proved that any closed
itemset is a ppc extension of the other unique closed itemset.
Thus, the binary relation ppc extension induces a rooted tree whose root
is C(emptyset). Thus, starting from the smallest closed itemset (which
is usually empty set) and find ppc extension recursively for all i, we
can perform depth first search on the rooted tree. The computation of 
closure of P can be done by taking the intersection of transactions
including P, thus it can be done by occurrence deliver, in the linear
time of T(P).

   ====   sweep pointer method ====
Actually, we do not have to compute the intersection completely. We 
choose one transaction T among them, and check whether there is an item 
in T\setminus P which is included in all the other transactions.
We trace T in the increasing order of items, and when we meet an item i
in T\setminus P, we trace the other transactions from the head whether they
include i or not. Notice that to check the next item j in T\setminus P,
we can start the trance of the other transactions from the positions 
which we terminated the trace for checking i. This does not increase the 
computation time much since the time complexities are the same, and 
usually we check only few items and conclude that the itemset is not
a child. Thus, we can do ppc check very quickly.

  =====  bit-mask method  ======
Moreover, when the frequency of the itemset is small, we can use bit
operations so that we can operate 32 or 64 bits at once.
Suppose that the frequency frq(I) of an itemset I is less than 32.
We give ID's 0,1,2,4,...,2^{frq(I)} to Occ(I), which is the set of occurrences
of I. Let the bit-mask b(S) of a subset S of Occ(I) be the sum of ID's of the
occurrences in S. For each item j included in at least one occurrence
of I, we compute b(Occ(I\cup {j})). When we compute the closure C(I\cup {j})
of I\cup {j}, an item e not in I\cup {j} is included in C(I\cup {j}) 
if and only if b(Occ(I\cup {e})) \cap b(Occ(I\cup {j})) = b(Occ(I\cup {j})).
Here \cap means the and operation for bit string, and two numbers are 
regarded as bit-strings. This operation can be done in very few steps, thus
we can accelerate the closure computation. If I\cup {j} is a closed itemset,
we can use the bit-masks for the ppc extensions of I\cup {j}. Thus 
we do not have to re-compute the bit-masks in the recursive calls. In practice,
the bit-mask method accelerates the speed of the algorithm 2 or 3 times when 
the data set is sparse and the minimum support is small, say 10.

  ====  recursive pruning for closed itemset ===
For the efficient computation of closed itemsets, we have one more technique.
Suppose that for a closed itemset I and an item j, C(I\cup {j}) is not a ppc 
extension of I, because C(I\cup {j}) includes an item e<j, not in I.
Then, we can see for any ppc extension I\cup {j'}, j'<j, C(I\cup {'j}\cup {j})
includes item e, thereby C(I\cup {'j}\cup {j}) is not a ppc extension of
I\cup {j'}. It implies that we can never get a ppc extension by adding j,
in the recursion. This reduces the number of candidates of ppc extensions,
so reduce the computation time.
In practice, when the difference between the number of frequent closed
itemsets and the number of the frequent itemsets is large, it speeds the 
algorithm about 2 or 3 times.


###############################
####    Acknowledgments    #### 
###############################

We thank to Ken Satoh of National Institute of Informatics Japan,
Hiroki Arimura of Hokkaido University for their contribution for the research,
to Tatsuya Asai, Yuzo Uchida, Masashi Kiyomi for their contribution for 
computational experiments and coding, and to Ilpo Lyytinen of University
of Helsinki for their contributions to fix bug. 
We would also like to present our sincere thanks to Bart Goethal, one of 
the organizers of FIMI (Frequent Itemset Mining Implementation) to 

A part of the research of LCM is supported by joint-research fund of
National Institute of Informatics Japan, and Grant-in-aid from the
Ministry of Education, Science, Sport and Culture of Japan
(Monbu-Kagaku-Sho).


##########################
####    References    #### 
##########################

Bart Goethal, "FIMI repository", http://fimi.cs.helsinki.fi/
(various frequent itemset implementation, benchmark databases, computational
experiments comparing all implementations, and papers explaining the
implementations, very useful site)

Takeaki Uno, Masashi Kiyomi, Hiroki Arimura, LCM ver.3: Collaboration of Array, Bitmap and Prefix Tree for   Frequent Itemset Mining, Open Source Data Mining Workshop on Frequent Pattern Mining Implementations 2005, Aug/2005

Takeaki Uno, Masashi Kiyomi, Hiroaki Arimura, "LCM ver.2: Efficient Mining Algorithms for Frequent/Closed/Maximal Itemsets," in Proceedings of IEEE ICDM'04 Workshop FIMI'04, 1/Nov/2004, http://sunsite.informatik.rwth-aachen.de/Publications/CEUR-WS//Vol-126/

Takeaki Uno and Tatsuya Asai, Hiroaki Arimura and Yuzo Uchida, "LCM: An Efficient Algorithm for Enumerating Frequent Closed Item Sets," Workshop on Frequent Itemset Mining Implementations (FIMI'03), http://sunsite.informatik.rwth-aachen.de/Publications/CEUR-WS//Vol-90/

Takeaki Uno,Tatsuya Asai,Yuzo Uchida, Hiroki Arimura, "An Efficient Algorithm for Enumerating Closed Patterns in Transaction Databases," Lecture Notes in Artificial Intelligence 3245 (Proceedings of Discovery Science 2004), 4/Oct/2004 


