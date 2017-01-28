-- upper triangular matrix offset calculation

uoffset i j =  (n * (n + 1) - (n - i) * (n - i + 1) )/2 + j - i
    where
    n=1000 

offset i j | i <= j    = uoffset i j
           | otherwise = -1

size n = n * (n+1) / 2
