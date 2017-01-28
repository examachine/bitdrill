module Main where
import System
import IO
import Graph
import Graph_IO
import Hypergraph

process fname1 = 
    do putStrLn ("reading F2 graph " ++ fname1)
       h1 <- openFile fname1 ReadMode
       graph <- loadMetis h1
       putStrLn "read graph"

main = do putStrLn "test comms model"
	  args <- getArgs
	  case args of
		    [fname1] -> process fname1
		    _ -> putStrLn "usage: test-comms-model filename"
