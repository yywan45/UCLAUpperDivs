FILE DESCRIPTIONS -------------------------------------

README
Current file containing file descriptors and answers to questions detailed in spec.

Makefile
Builds the executables, runs the tests and produces the graphs through gnuplot. Also cleans the directory and makes the submission tarball.

lab2_list.c
Source code of program. Porgram drives one or more parallel threads to execute operations on a shared linked list, and outputs details about the final list and the performance of the operations. Program can take five options (thread, iteraions, yield, sync, lists). Yield option has i, d, l options, corresponding to linked list insertion, deletion and lookup. Sync has m, s options, corresponding to different locked versions of linked list functions.

SortedList.h
Header file that contains all of function declarations for doubly linked list.

SortedList.c
Source code of implementations of functions in SortedList.h.

lab2_list.sh
Script that runs performance tests.

lab2b_1.png
lab2b_2.png
lab2b_3.png
lab2b_4.png
lab2b_5.png
Plots from data from performance tests.

lab2b_list.csv
Results of test from performance tests.


QUESTIONS -------------------------------------

2.3.1
I believe most of the cycles spend in the 1 and 2-thread tests are in initializing the list and the timer. Since there are very little threads, the critical section of the code will execute pretty smoothly since there will be less waiting for locks to be released. Most of the time/cycles for the high-thread spin-lock tests are spent in the critical section of the code, where not more than one thread can be executed. Since there are many threads, and the spin lock will have to keep spinning and wait for the lock to be released when another thread finishes running through the section, there will be many cycles spent spinning before all the threads can go through the critical section. Most of the time/cycles for the high-thread mutex tests are spent in the critical section of the code, where not more than one thread can be executed. Since there are many threads, and the mutex lock goes to sleep when it cannot access the critical section, the overhead from sleeping and waking up of the threads will be very expensive, perhaps even more so than spin-locks.

2.3.2
The code in the critical section is consuming the most cycles when the spin-lock version of the list exerciser is run with a large number of threads. This operation becomes expensive with large number of threads due to the handling of the locks as it is passed between threads so that they can have their turn to run through the critical section.

2.3.3
The average lock-wait time rises with the number of contending threads because as the number of threads increases, then there are more threads contending for the lock when it is released. Scheduling the threads to get the lock will create more overhead when there are more threads. Thus, the average lock-wait time will increase with the number of threads. The completion time per operation also increases with the increasing number of contending threads because there will be more time spent waiting, thus more time it takes before the operations will complete. The wait time per operation goes up faster than the completion time per operation because the completion time is also based on other factors such as initialization, and will be less susceptible to the increase in time due to increasing number of threads, while the wait time for locks will increase directly based on the number of threads.

2.3.4
The performance becomes better with increasing number of lists because the program becomes more parallelized. As the number of lists increase, then the threads have less contention for the locks as there are a lower amount of threads per list, leading to a decreased cost per operation. The throughput (operations per second) will increase until there is a one to one ratio between the threads and the lists, then more increases in the number of list will not increase the throughput since the number of lists will exceed the number of threads.

