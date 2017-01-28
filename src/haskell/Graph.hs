--module Graph(Graph,mkGraph,adjacent,nodes,edgesU,edgesD,edgeIn,
--                   weight) where
module Graph where
import Array

graph :: (Ix n,Num w) => Bool -> (n,n) -> [(n,n,w)] -> (Graph n w)
adjacent :: (Ix n,Num w) => (Graph n w) -> n -> [n]
nodes :: (Ix n,Num w) => (Graph n w) -> [n]
edgesU :: (Ix n,Num w) => (Graph n w) -> [(n,n,w)]
edgesD :: (Ix n,Num w) => (Graph n w) -> [(n,n,w)]
edgeIn :: (Ix n,Num w) => (Graph n w) -> (n,n) -> Bool
weight :: (Ix n,Num w) => n -> n -> (Graph n w) -> w

-- Adjacency list representation --

type Graph n w = Array n [(n,w)]

graph dir bnds es =
    accumArray (\xs x -> x:xs) [] bnds 
               ([(x1,(x2,w)) | (x1,x2,w) <- es] ++
                if dir then []
                else [(x2,(x1,w))|(x1,x2,w)<-es,x1/=x2])

adjacent g v    = map fst (g!v)

nodes g         = indices g

edgeIn g (x,y)  = elem y (adjacent g x)

weight x y g    = head [ c | (a,c)<-g!x , a==y]

edgesD g        = [(v1,v2,w) | v1<- nodes g , (v2,w) <-g!v1] 

edgesU g        = [(v1,v2,w) | v1<- nodes g , (v2,w) <-g!v1 , v1 < v2] 

graphAL = array (1,5) [(1,[(2,12),(3,34),(5,78)]),
                       (2,[(1,12),(4,55),(5,32)]),
                       (3,[(1,34),(4,61),(5,44)]),
                       (4,[(2,55),(3,61),(5,93)]),
                       (5,[(1,78),(2,32),(3,44),(4,93)])]

-- same graph but created with mkGraph but different ordering of arcs
graphAL' = graph False (1,5) [(1,2,12),(1,3,34),(1,5,78),
                              (2,4,55),(2,5,32),
                              (3,4,61),(3,5,44),
                              (4,5,93)]

-- End of Adjacency list representation
