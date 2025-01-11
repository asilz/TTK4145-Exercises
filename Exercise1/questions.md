Exercise 1 - Theory questions
-----------------------------

### Concepts

What is the difference between *concurrency* and *parallelism*?
With parallelism we have multiple tasks running simultaneously. With concurrency, we are "juggling" between multiple tasks. This means that the tasks are not necessarily running simultaneously, but we are rather constantly switching between them.

What is the difference between a *race condition* and a *data race*? 
Race condition: When the result of an operation is dependent on timing
Data race: When a thread accesses a variable while another thread is writing to it

*Very* roughly - what does a *scheduler* do, and how does it do it?
It controls which threads are run by using for example interrupts. It can use different scheduling algorithms such as round robin to ensure all tasks are completed

### Engineering

Why would we use multiple threads? What kinds of problems do threads solve?
> Having multiple threads improves the performance of the program by allowing the operating system to use multiple cores to run the program, Threads help solve problems where multiple tasks can run somewhat independently without heavily depending on the results of each other. 

Some languages support "fibers" (sometimes called "green threads") or "coroutines"? What are they, and why would we rather use them over threads?
Instead of having multiple threads, we can have a single thread switch between different functions. This means we can run multiple functions concurrently while not having to deal with costly thread context-switching or having to allocate a stack for each thread. 

Does creating concurrent programs make the programmer's life easier? Harder? Maybe both?
It depends on the requirements of the programming project. If it is intuitive to seperate the code into multiple concurrent parts, then it can make the code easier to read. It also allows the OS to use multiple threads for a single program.

What do you think is best - *shared variables* or *message passing*?
Message passing is nice when we have a system with multiple devices that do not share memory, otherwise I prefer shared variables as they are usually faster and require less involvment from the operating system (potentially fewer syscalls)


