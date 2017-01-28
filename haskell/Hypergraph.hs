module Hypergraph where
import FiniteMap
import Set

-- A hypergraph is a family of sets which are subsets of a vertex set X
-- This implementation uses an edge list representation that
-- directly corresponds to the mathematical definition
-- n index type, w weight type
-- type Hypergraph n w = Array n [(n,w)]

-- show functions for set and finitemap

instance (Show a) => Show (Set a) where   
    show set = show (setToList set)

instance (Show a, Show b) => Show (FiniteMap a b) where   
    show vs = show (fmToList vs)

type HEdges l = FiniteMap l (Set l)

data Hypergraph l = Hypergraph (HEdges l) (HEdges l) deriving Show

-- hgraph constructor
-- takes a list of edges
hgraph list = let edges = hEdges list in
		  Hypergraph edges (dualEdges edges)

-- hgraph edge_list
--hEdges :: [ [a] ] -> HEdges a
hEdges el = listToFM (zip [1..length el] (map mkSet el))
--     = Hypergraph (listToFM . mkSet (edge_list)) (listToFM . mkSet (edge_list))

-- compute dual of a given hgraph

dualEdge (edgeIx, hEdge) = map (\x->(x, mkSet [edgeIx])) (setToList hEdge)

dualEdges hEdges = ( (addListToFM_C union emptyFM) . concat .
		     (map dualEdge) . fmToList )
		     hEdges
--hVertices el = (zip (map mkSet el) [1..length el])

-- hyper vertices are the dual of the hypergraph
--hVertices hEdges = (foldFM uni emptyFM). (mapFM dual)) hEdges
--		   where uni = plusFM_C 
--dual hEdges = ( listToFM . map (\(x,y)->(y,x)) . fmToList) hEdges

