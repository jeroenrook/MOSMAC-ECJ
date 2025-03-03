Author: Yasha Pushak
Created: 2018-10-30

In our original PPSN 2018 paper "Algorithm configuration landscapes: More 
benign than expected?" that used this scenario, we used a different version of
the circuit fuzz instances (available from AClib version 1), that contained a 
superset of the instances here as the test set. 

The training set was the same, and the test set here should be a random sample
from the set we used. So we expect any work done to replicate our results with
this scenario should provide an unbiased approximation of the results we 
obtained. However, we would expect the confidence intervals to be larger, since
the size of the test set here is smaller.

You can find more on our work here: http://www.cs.ubc.ca/labs/beta/Projects/ACLandscapes/


