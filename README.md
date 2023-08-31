Author: Michael Chavrimootoo

# Overview



# Structure

In this folder are the following files:
- lib.h: the implementations for data structures and election systems
- sampling.h: the implementations for the sampling procedures
- main.cpp: a driver file for the sampling program
- README.md (this file)
- input0 : small test input with random data
- input1 : small test input with a uniform distribution (dispersions = 1)
- input2 : small test input with low probability of winning for p
- input3 : a larger test input with random data
- TODO: Add a fun chart
- TODO: Add unit tests
- TODO: Add code coverage
- TODO: Add Makefile

# Running the Code

To compile the code do:

g++ -std=c++11 main.cpp

And run using:

./a.out < inputX

where X is in {0, 1, 2, 3} i.e. input is read form STDIN. You can use your own file. The test files are formatted as such:
c n p // c is the number of candidates, n is the number of votes, p is the name of the candidate you want to win
v1 v2 ... vc       // n such lines
epsilon delta

# Understanding the Outputs

The output is the probability estimate using the Monte Carlo method and the estimate using the Moser-Tardos method.

# Evaluation

The code runs quite quickly: a few second for input2 and input3 and less for input0 and input1.
