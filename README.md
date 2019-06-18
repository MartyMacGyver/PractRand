# PractRand
A fork /mirror of the Practically Random suite of statistical tests & psuedo-random number generators (RNGs, PRNGs)

Based on PractRand from https://sourceforge.net/projects/pracrand/

## Notes from latest

FYI, `bin/` and `lib/` has MSVS 2012 x86 and x64 binaries from the original authors as above

Building: The free Microsoft Visual Studio 2019 works fine for Windows. I don't plan to distribute my own binaries or updated project files at this time though, but it worked well when I tried it.

## My notes from v0.92

I've been able to build in Linux (Ubuntu) and Windows equally well.

For Windows I used the MinGW-W64 5.3.0 compiler from https://sourceforge.net/projects/mingw-w64/ (I selected the "x86_64", "posix", and "seh" options during installation). I gave this all a try with the 6.1.0 tools as well - seems to work fine too.

Once you have gcc/g++ working, run the following from the command line in the root dir of this project:

```
g++ -c src/*.cpp src/RNGs/*.cpp src/RNGs/other/*.cpp -O3 -Iinclude -pthread -std=gnu++11
ar rcs libPractRand.a *.o
g++ -o RNG_test tools/RNG_test.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
g++ -o RNG_benchmark tools/RNG_benchmark.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
g++ -o RNG_output tools/RNG_output.cpp libPractRand.a -O3 -Iinclude -pthread -std=gnu++11
```

A library and the expected executables are produced.
