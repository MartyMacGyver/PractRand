# PractRand
A fork of the Practically Random suite of statistical tests &amp; psuedo-random number generators (RNGs, PRNGs)

Based on PractRand from https://sourceforge.net/projects/pracrand/

Building:
It looks like MSVC >= 2012 should work for Windows. I don't plan to create binaries at this time though, and prefer to use platform-agnostic tools where possible.

I've been able to build in Linux (Ubuntu) and Windows equally well (using MinGW-W64 5.3.0+ for Windows) as follows:

Once you have gcc/g++ working, run the following from the command line in the root dir of this project:

```
g++ -c src/*.cpp src/RNGs/*.cpp src/RNGs/other/*.cpp -O3 -Iinclude -pthread -std=gnu++11
ar rcs libPractRand.a *.o
g++ -o RNG_test tools/RNG_test.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
g++ -o RNG_benchmark tools/RNG_benchmark.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
g++ -o RNG_output tools/RNG_output.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
```

