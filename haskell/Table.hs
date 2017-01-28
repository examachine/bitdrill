module Table(Table,newTable,findTable,updTable) where

{--- general index ---}
newTable    :: (Eq b) => [(b,a)] -> Table a b
findTable   :: (Eq b) => Table a b -> b -> a
updTable    :: (Eq b) => (b,a) -> Table a b -> Table a b

{-- Function implementation --

newtype Table a b   = Tbl (b -> a)

instance Show (Table a b) where
    showsPrec _ _ str = showString "<<A Table>>" str

newTable assocs = foldr updTable 
                        (Tbl (\_ -> error "item not found in table"))
                        assocs

findTable (Tbl f) i   = f i

updTable (i,x) (Tbl f) = Tbl g
    where g j | j==i      = x
              | otherwise = f j
-- end of Function implementation --}

{-- List implementation --}

newtype Table a b        = Tbl [(b,a)]
    deriving Show

newTable   t          = Tbl t

findTable (Tbl []) i = error "item not found in table"
findTable (Tbl ((j,v):r)) i
     | (i==j)        = v
     | otherwise     = findTable (Tbl r) i 

updTable e (Tbl [])         = (Tbl [e])
updTable e'@(i,_) (Tbl (e@(j,_):r))
     | (i==j)         = Tbl (e':r)
     | otherwise      = Tbl (e:r')
     where Tbl r' = updTable e' (Tbl r)

{-- end of List implementation --}

{--- end of general index ---}

{--- Array implementation ---

import Array

newTable    :: (Ix b) => [(b,a)] -> Table a b
findTable   :: (Ix b) => Table a b -> b -> a
updTable    :: (Ix b) => (b,a) -> Table a b -> Table a b

newtype Table a b     = Tbl (Array b a)
    deriving Show

newTable l = Tbl (array (lo,hi) l)
    where
           indices = map fst l
           lo      = minimum indices
           hi      = maximum indices

findTable (Tbl a) i      = a ! i

updTable p@(i,x) (Tbl a) = Tbl (a // [p])

--- end of Array implementation ---}


