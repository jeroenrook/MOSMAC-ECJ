These were created using Armin Biere's cnf fuzzer fuzzsat, which generated circuits and translates them into CNFs.
We generated many circuits (using the options -i 100 -I 100) and then filtered out all that could be solved by lingeling in < 1 second (on one particular desktop computer).
