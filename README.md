# 2402MC06_OSlab_Asg06


To build and execute the programs, first replace all the modified files(I've included only the modified files here) in the xv6 source tree, then run make qemu, and then in the terminal run the program name as a command.

NOTE: These files are compatible with the MacOS port of xv6 by iitb, and the compiler has been changed from 1386-gcc to 1686-gcc for convenience, so these implementations might not work on the x86 version of xv6.



Q1.
Implemented the bankers algorithm to handle resource requests for shared resources. This consists of two parts, the first one being resource request and allocation part and the other one is the safety check. When a new request comes, the algorithm pretend allocates the resources to the process if available and then runs a safety check to see if the process table is in a safe state or not by simulating the worst case scenario and seeing if the processes would successfully execute or not.



Q2.
Implemented a deadlock detection algorithm by using a wait for graph. The wait for graph has process as vertices and an edge exists between Pi -> Pj if Pi is waiting for a resource held by Pj. In this graph, if a cycle arises at some point(detected by DFS), then we can say the system is in a deadlock.



Q3.
Implemented Deadlock prevention by resource ordering. In this we can prevent deadlocks by defining a global order for all available resource in which the locks for it must be obtained. Now, if tow processes are competing for two resources, they cannot hold locs for the two of them simultaneously. Semaphores were used for the locks.


Q4.
implemented multi-resource synchronisation. The shared resources were defined in a shared page table along with its own counting semaphores to handle its allocation. To prevent deadlocks, global resource ordering was used.
