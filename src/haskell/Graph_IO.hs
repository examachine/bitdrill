module Graph_IO where
import IO
import Graph
import Array
import IO_Util
import List

loadMetis :: Handle -> IO (Graph Int Int)
loadMetis h = do str1 <- hGetLine h
		 edgeLines <- hGetLines h
		 let params = valList str1 :: [Int] 
		     adjlist = zip [1..length edgeLines]
			           (map (mkadj.valList) edgeLines)
		     mkadj l = map (\x-> (x,1)) l
		     in
		     return (array (1, head params) adjlist)
