FILE DESCRIPTIONS -------------------------------------

README
Current file containing file descriptors and answers to questions detailed in spec.

Makefile
Builds the executables, runs the tests and produces the graphs through gnuplot. Also cleans the directory and makes the submission tarball.

lab2_add.c
Source code for part 1. Program spawns threads and add/subtract 1 from a counter for specified iterations. Program can take four options (thread, iterations, yield, sync). Yield option has m, s, c options, corresponding to different versions of original add function given in spec.

lab2_list.c
Source code for part 2. Program spawns threads and adds/removes an element from doubly linked list for specified iterations. Program can take four options (thread, iteraions, yield, sync). Yield option has i, d, l options, corresponding to linked list insertion, deletion and lookup. Sync has m, s options, corresponding to different locked versions of linked list functions.

SortedList.h
Header file that contains all of function declarations for doubly linked list.

SortedList.c
Source code of implementations of functions in SortedList.h.

lab2_add.sh
Test cases obtained from piazza page for lab2_add executable. Also contains command to gnuplot results of lab2_add.csv using given lab2_add.gp file.

lab2_list.sh
Test cases obtained from piazza page for lab2_list executable. Also contains command to gnuplot results of lab2_list.csv using given lab2_list.gp file.


lab2_add-1.png
lab2_add-2.png
lab2_add-3.png
lab2_add-4.png
lab2_add-5.png
Plots obtained by using gnuplot and lab2_add.gp file. More details in plot and axes titles of each plot.


lab2_list-1.png
lab2_list-2.png
lab2_list-3.png
lab2_list-4.png
Plots obtained by using gnuplot and lab2_list.gp file. More details in plot and axes titles of each plot.

lab2_add.csv
Results of test from running commands from lab2_add.sh.

lab2_list.csv
Results of test from running commands from lab2_list.sh.


QUESTIONS -------------------------------------

2.1.1
There needs to be enough iterations where the program's unprotected operations can cause a context switch when updating the counter, leading to the program failing.
When there are smaller number of iterations, there is less time to overlap the execution of updating the counter, so there is less chance of the program failing.

2.1.2
The overhead from calling the pthread_yield() function causes the --yield runs to be much slower. The additional time goes to the 'context switch' necessary to yield one thread and let another thread run on the CPU. It is possible to get valid per-operation timings when using --yield if we account for the overhead caused from calling pthread_yield(). However, since the behavior of this function varies at different calls, it might not be very accurate to simply account for the overhead, and this method might be invalid.

2.1.3
The average cost per operation drops with increasing iterations because the overhead time is split across increasing iterations, decreasing the average.
We can know the 'correct' cost by accounting for the overhead at the beginning when allocating memory for the threads before calculating the cost per iteraion.

2.1.4
With less threads, there is less opportunities for blocks to occur so the threads spend less time being blocked. The protected and unprotected versions thus have a negligible difference in run time.
The three protected operations will slow down as number of threads increases because they are less able to execute due to there being more blockages from the increasing number of threads.
Spin-locks are expensive for large number of threads because the spin-lock operation does not sleep when it is blocked, compared to the mutex that goes to sleep and lets another thread run. THe CPU is thus strained with spin-locks and leads to an increase in cost per operation with spin-locks.

2.2.1
Part 2's time per protected operation vs the number of threads is more than part 1 because part 1 changes the value of a variable, while part 2 is a growing linked list. Thus, part 2 will have more operations compared to part 1. So, with part 2, more threads will be blocked due to more operations, which might create a bottleneck as the program is running through the critical section.

2.2.2
Spin locks' time per protected operation vs the number of threads is less than mutex locks. This is because mutex locks go to sleep when it lets another thread run and this can waking up can lead to some overhead. This is as opposed to spin locks that will constantly try to run.


