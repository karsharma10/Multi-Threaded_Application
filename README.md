# Multi-Threaded_Application in C:


This project encompasses a multi-threaded application written in C, designed to efficiently resolve domain names into their corresponding IP addresses. This process mirrors the fundamental operation that takes place every time a user accesses a new website via a web browser.

The architecture of this application centers around two distinct types of worker threads: Requesters and Resolvers. These threads seamlessly interact via a Shared Stack, a bonded buffer that facilitates communication between them.

Each input file is meticulously processed using a single requester thread per file. These threads read files line by line, extract hostnames from the shared array, perform IP address lookups, and subsequently write the outcomes to the resolver logfile.










