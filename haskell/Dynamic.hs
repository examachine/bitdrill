module Dynamic(module Table,dynamic) where
-- to get access to the imported modules in Hugs do
-- :set -P../Chapter5:{Hugs}/lib:{Hugs}/lib/hugs:{Hugs}/lib/exts
import Table

dynamic :: Ix coord => (Table entry coord -> coord -> entry)
                        -> (coord,coord) -> (Table entry coord)
dynamic compute bnds = t
    where t = newTable (map (\coord ->(coord,compute t coord)) 
                            (range bnds))
