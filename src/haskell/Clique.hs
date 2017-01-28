module Clique where
import Set
import Graph

maxCliques :: (Graph n w) -> [ (Set n) ]
maxCliques g = map (maxCliqueAt g) (range (bounds g))

-- find maximal cliques containing vertex u
maxCliqueAt g u
    = let cliques = mkSet [u]
	  adj = adjacent g u
	  in
	  [(x,y)|]
