module IO_Util where
import IO

lexList :: ReadS [String]

lexList "" = [ ( [], "") ]

lexList str = 
    [ ( (lexeme:rest), "") ] where
    [ (lexeme, rem) ] = lex str
    [(rest,_)] = lexList rem

type ReadSList a = [ ( [a], String ) ]
readsList :: (Read a) => ReadS [a]
readsList str = [ ( valList str, "") ]

--valList :: [a]
valList str = ( fst . unzip . concat . (map reads) ) lexlist where
 	      [(lexlist,_)] = lexList str

hGetLines :: Handle -> IO [String]
hGetLines h = do eof <- hIsEOF h
		 if eof then return [] else
		    do l1 <- hGetLine h
		       lrest <- hGetLines h
		       return (l1:lrest)
