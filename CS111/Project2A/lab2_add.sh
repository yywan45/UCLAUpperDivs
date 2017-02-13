#!/bin/bash

# range of threads and iterations to see what it takes to cause a failure
./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=100 >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 >> lab2_add.csv

# range of threads and iterations to cause failure with yield
./lab2_add --threads=2 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv

# look at cost per operation vs number of iterations
./lab2_add --threads=1 --iterations=100 >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000000 >> lab2_add.csv
./lab2_add --threads=1 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=1 --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000000 --yield >> lab2_add.csv

# use gnuplot
/u/cs/grad/zhou/iloveos/gnuplot lab2_add.gp lab2_add.csv
