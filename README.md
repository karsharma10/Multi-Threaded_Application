# Multi-Threaded_Application in C:


This project encompasses a multi-threaded application written in C, designed to efficiently resolve domain names into their corresponding IP addresses. This process mirrors the fundamental operation that takes place every time a user accesses a new website via a web browser.

The architecture of this application centers around two distinct types of worker threads: Requesters and Resolvers. These threads seamlessly interact via a Shared Stack, a bonded buffer that facilitates communication between them.

Each input file is meticulously processed using a single requester thread per file. These threads read files line by line, extract hostnames from the shared array, perform IP address lookups, and subsequently write the outcomes to the resolver logfile.

As the completion of file processing is achieved, requester threads gracefully conclude their tasks. Upon the successful resolution of all hostnames, resolver threads terminate, bringing the program to a close.


Central to the efficiency of this solution is the utilization of a thread-safe shared stack, addressing the bounded buffer problem. The shared stack leverages a contiguous, linear memory array (LIFO) to enhance performance.


To ensure synchronization, eliminate deadlock, and avoid busy waits, the application harnesses the power of mutexes and semaphores. This orchestration of resources aligns with best practices, fulfilling synchronization, deadlock prevention, and avoidance of resource starvation.

In essence, this multi-threaded DNS resolver showcases my proficiency in C programming, concurrent computing, synchronization mechanisms, and system-level problem-solving. Feel free to explore the codebase and its implementation details to gain a deeper understanding of this project.








