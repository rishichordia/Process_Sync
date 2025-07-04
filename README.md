#  Starve Free Readers Writers Problem
Submitted by: Pragya Dalal 19114063 

## Contents

1. [Introduction](#introduction) <br>
   a. [Problem Statement](#problem-statement) <br>
   b. [First Readers Writers](#first-readers-writers-problem)<br>
   c. [Second Readers Writers](#second-readers-writers-problem)
2. [Starve Free Implementation](#starve-free-implementation)
3. [Faster Starve Free ](#faster-starve-free)
4. [Steps To Run](#steps-to-run)
5. [Conclusion](#conclusion)
6. [References](#references)

## Introduction
The following documentation describes a starvation free solution to the readers writers problem.
### Problem Statement
The readers writers problem is a classical synchronization problem where a shared data structure can be modified simultaneously by concurrent threads.  The problem is described as:-
- There are a number of readers and writers running concurrently 
- There is a shared region of memory(critical section) 
- Only one writer may access critical section at a time
- When no writer is writing, any number of readers are allowed to read data 



The problem is to **synchronize the access to shared variables** in such a way that at a time only one operation is taking place. This is needed so as to avoid **false retrieval or corruption of data**  due to multiple persons accessing it at the same time. The solution involves implementing mututal exclusion of the shared region using a semaphore.


There are three variants of the solution:-
- First Readers Writers Problem
- Second Readers Writers Problem
- Third or Starve Free Readers Writers Problem
  
### First Readers Writers Problem
In this variant, the readers are given preference over the writers. The solution is such that:- 
- If readers are reading in the critical section, the writers wait untill all the readers have finished reading. 
- If some readers are in critical section, any new reader that comes after the writer,are given access and the writer has to wait.
- When no readers are either waiting or reading, the writers can write.
- when a writer is writing, no other writer or reader is allowed in the critical section.
#### Pseudo Implementation
Initially, `count_readers=0`, `reader_mut_exclusion = 1` and `resource = 1` .
#### Code for Readers
```c
while(true)
{   
    wait(reader_mut_exclusion);                     //Lock this section so that only one reader at a time can enter and change count_readers
        count_readers++;                            
        if(count_readers==1)                        //Check if this reader is the first reader
            wait(resource);                         //If this reader is the first reader then wait if any writer is still writing and then lock the resource, subsequent readers don't need to relock it 
    signal(reader_mut_exclusion);                   //Release the lock for other readers

    /* 
        -----
    DO READING
        -----
    */

    wait(reader_mut_exclusion);                     //Lock this section so that only one reader at a time can enter and change count_readers
        count_readers--;                            
        if(count_readers==0)                        //Check this is the last reader 
        signal(resource);                           //If this is the last reader, then unlock the resource so that now it is available to writers                        
    signal(reader_mut_exclusion);                   //Release the lock for other readers
}
```
#### Code for Writers
```c
while(true){
    wait(resource);                                 //Lock this shared area so that only this writer can write at this time
        /* 
            -----
        DO WRITING
            -----
        */
    signal(resource);                               //Unlock this area so that if there is a reader requesting for it can take it or if no reader is waiting then writer can access this 
}
```

This solution works well in terms of mutual exclusion but there is UNBOUNDED WAIT. The writers may starve to enter the critical section indefinitely as readers might keep coming.Hence there is **starvation of writers**.

### Second Readers Writers Problem
In the second variant, the writers are given more preference as compared to the readers. The solution is such that:-
- Any reader in the critical section,releases the mutex lock if there is a writer waiting.
- Once a writer is in the critical section, it writes the data and allows any number of writers to write inspite of readers waiting to read.
- Once all the writers have written, readers are allowed to access the data.
#### Pseudo Implementation
Initially, `count_readers = 0`, `count_writers = 0`,`reader_mut_exclusion = 1`, `reader_mut_exclusion = 1`, `reader_trying_enter = 1` and `resource = 1` .
#### Code for Readers
```c
while(true)
{ 
    wait(reader_trying_enter);              //Readers trying to enter the critical section wait over this semaphore             
        wait(reader_mut_exclusion);         //Lock this section so that only one reader at a time can enter and change count_readers         
        count_readers++;                 
        if (count_readers == 1)             //If the current reader is first one
            wait(resource);                 //Then wait for previous writers to finish and then lock the resource      
        signal(reader_mut_exclusion);       //Release Lock                 
        signal(reader_trying_enter);        //Release Lock

    /* 
        -----
    DO READING
        -----
    */

    wait(reader_mut_exclusion);             //Lock this section so that only one reader at a time can enter and change count_readers          
        count_readers--;                 
        if (count_readers == 0)             //Check if you are the last reader
            signal(resource);               //If yes,then unlock the resource for writers 
    signal(reader_mut_exclusion);           //Release Lock
}
```
#### Code for Writers
```c
while(true){
    wait(writer_mut_exclusion);             //Lock this section so that only one writer at a time can enter and change count_writers         
        count_writers++;                    
        if (count_writers == 1)             //Check if we are the first writer
            wait(reader_trying_enter);      //If yes, Block new readers form entering into their entry section 
    signal(writer_mut_exclusion);  

    wait(resource);                         //Now wait for the readers that arrived before writer to finish reading and then block the access to resource
                                            //This ensures that only one writer is in the critical section at a time
    /* 
        -----
    DO WRITING
        -----
    */

    signal(resource);                       //When the writer finished,it can release the resource for other readers and writers to access the critical section

    wait(writer_mut_exclusion);             //Lock this section so that only one writer can enter at a time and change the value of count_writers
        count_writers--;                
        if (count_writers == 0)             //Check if there are no more waiting writers and this is the last writer
            signal(reader_trying_enter);    //If yes, allow readers to procedd to critical section            
    signal(writer_mut_exclusion);           //Release Lock
}
```
This solution is preferred in applications where writes are more important than reads.However,there is a flaw. Some readers might keep waiting for the resources indefinitely as more and more writers come. Hence there is **starvation of readers**.

## Starve Free Implementation
A starve free implementation is where none of the readers or the writers have to wait indefinitely to access the shared resources.The main idea behind this solution is to add the contraint that threads wait only for a bounded amount of time. An easy way in which we can avoid starvation is by controlling the order in which readers and writers in the waiting queue access the critical section. The waiting threads are executed in the order of their arrival.

### Pseudo Implementation
In this starve free implementation,we have used a custom sempahore. This is because it is necessary for the semaphore to have a waiting queue with FIFO wakeup.The pseudo code is as follows:
```C
struct semaphore
{
    int value;
    pthread_mutex_t lock;   //Mutex Lock 
    Queue *waitingQueue;    //Waiting Queue

    void semaphore_wait()
    {
        mutex_lock(&lock);
        value--;
        
        if(value<0)
        {
            waitingQueue->push(getpid());
            block();
        }
        mutex_unlock(&lock);
    }

    void semaphore_signal()
    {
        mutex_lock(&lock);
        value++;

        if(value <= 0)
        {
            int frontId=waitingQueue->pop();
            wakeup(frontId);
        }
        mutex_unlock(&lock);
    }
};
```
A queue is used to keep track of the order in which the processes arrrive so that they get executed in the same `FIFO` order.

The above pseudo-code uses a Queue for this purpose .However for threads, this can be imitated in linux using a condition variable along with `pthread_cond_wait()` and `pthread_cond_signal()` system calls which has been used in the code implementation.
### Code Implementation 
In the implemented code, `Semaphores` are used for mutual exclusion.There are three semaphores used:-
1. `resourceAccess`:- This is a mutex lock used to access the shared resourc  e or enter the critical section.This semaphore 
2. `readerMutex`:- This semaphore is used to synchronize access of the readCount variable which is shared among the threads.The readCount keeps a count of the number of waiting readers.
3. `orderQueue`:- This is the most important semphore .It is the one used to remember the order of arrival of the incoming threads in a queue. It forces them to wait in a line to be serviced.

These semaphores are used to achieve synchronization.These semaphores have been manually implemented and have a `FIFO wakeup` of the threads waiting in a queue.
#### Intial Conditions
Initially `resourceAccess=1`. `readersMutex=1`, `orderQueue=1`,`shared_variable=0`,`readcount=0`.

#### Code for Writers
```c
void *writer(void* writer_number)
{   
    int num=*(int*)writer_number;
   /*ENTRY SECTION*/
    semaphore_wait(&orderQueue);          //Wait in line for turn 
    semaphore_wait(&resourceAccess);      //Request exclusive access to the shared resource
    semaphore_signal(&orderQueue);        //Allow others waiting in the queue to proceed.

   /*CRITICAL SECTION*/
    shared_variable+=2;
    printf("Writer %d modified value of shared variable to %d\n",num,shared_variable);
    
    /*EXIT SECTION*/
    semaphore_signal(&resourceAccess);    //Release the access to resource for next Reader/Writer

}
```
The writer checks the waiting queue of the `orderQueue` semaphore and waits untill its turn. Then it waits over the `resourceAccess` semaphore for the critical section to become free. Once it gains access, it updates the shared_variable and releases the critical section.This code is straightforward.

#### Code for Readers
```c
void *reader(void *reader_number)
{   
    int num=*(int*)reader_number;
    /*ENTRY SECTION*/
    semaphore_wait(&orderQueue);       //Wait for its turn in line
    semaphore_wait(&readerMutex);      //Gain mutex lock on readcount
    readcount++;
    if(readcount==1)                   //If there are currently no readers 
      semaphore_wait(&resourceAccess); //Request access to critical section
    semaphore_signal(&orderQueue);     //Allow others in line to execute
    semaphore_signal(&readerMutex);    //Release the mutex lock on readcount 

   /*CRITICAL SECTION*/
    printf("Reader %d: read the shared variable as %d\n",num,shared_variable);
   
   /*EXIT SECTION*/
    semaphore_wait(&readerMutex);      
    readcount--;
    if(readcount==0)                       //Check if the current reader is last one waiting in the arrival queue
        semaphore_signal(&resourceAccess); //If yes,then release access to critical section for writers 
    semaphore_signal(&readerMutex);        //Release the mutex lock on readcount
}
```
In the above implementation, the readers that arrives, waits for the writers to free the critical section.Then once they have acquired the critical section, keep on reading till the next writer doesnt arrive. With the help of orderQueue semaphore, the threads are run in the order they arrive.

Hence the above implementation of readers writers problem is found to be sarvation free. Also :- 
- A deadlock at `orderQueue` is **not possible** because it is requested only when no other semaphore is held by the calling thread.Hence in no condition can a deadlock occur.


**Note** It is very important that the semaphore_signal() wakes up the suspended or waiting threads in a FIFO manner.This is **not supported** in the convention **sem_post()** system call provided by linux in C. Hence I have implemented a custom semaphore with FIFO queueing.

## Faster Starve Free
The above solution is simple and fair enough but we have to use two semaphores for a reader to gain access of critical section in order of arrival of the threads. However, the mutex lock is a heavy system call and lowers the speed of execution, we therefore try to use reduce the number of semaphores used for accessing the critical section.

In this solution, there are 3 semaphores used:-
1. `in` :- Controls the readers getting into the critical section
2. `out` :- Used for mutual exclusion of the num_readers_out shared variable.
3. `writerAccess` :- THis semaphore is used to grant a waiting writing access to the critical section when the last reader in queue has finished.

#### Initial Condition
Initially, `in=1`,`out=1`,`writerAccess=0`,`num_readers_in=0`,`num_readers_out=0`,`shared_variables=0`.
#### Code for Readers

```C
void *reader(void *reader_no)
{   
   /*ENTRY SECTION*/
    semaphore_wait(&in);                     //Wait in queue for writers and readers to finish
    num_readers_in++;                            //Increment readers going in
    semaphore_signal(&in);                   //Signal waiting readers/writers

    /*CRITICAL SECTION*/
    printf("Reader %d: read shared variable as %d\n",*((int *)reader_no),shared_variable);

    /*EXIT SECTION*/
    semaphore_wait(&out);                    //Gain Access to the readers-out variable
    num_readers_out++;
    if(writer_wait==1&&num_readers_in==num_readers_out) //Here we check whether a writer is waiting ot not and current reader is last in queue
        semaphore_signal(&writerAccess);     //If yes, then grant access to a writer waiting
    semaphore_signal(&out); 
}
```
The readers sole access to the critical section is just controlled by the **in** semaphore.
- If there is a writer before the reader, then the writer has locked **in** semaphore .The reader waits for the writer to finish and then enters the critical section.
- If there are no writers before then the reader is given access to the critical section directly.
-  In the exit section, readers check whether writers are waiting or not. If they are the last reader, then they release the **writerAccess** semaphore and grant access to writers to enter critical section.

#### Code for Writers
```C
void *writer(void *writer_no)
{   
   /*ENTRY SECTION*/
    semaphore_wait(&in);                //This sempahore ensures that readers wait for this writer to finish 
    semaphore_wait(&out);               //Gain Access to num_readers_out
    
    if(num_readers_in==num_readers_out) //Checks if there are no readers in critical section
        semaphore_signal(&out);         //If yes, enter critical section and  num_readers_out variable 
    else
    {
        writer_wait=true;                  //If no, set boolean to true 
        semaphore_signal(&out);         //Release num_readers_out variable
        semaphore_wait(&writerAccess);  //Writer then waits for the readers waiting before it to finish 
        writer_wait=false;                  //Once finished,it enters the critical section and sets wait to false.
    }

   /*CRITICAL SECTION*/
    shared_variable+=2;
    printf("Writer %d modified shared variable to %d\n",(*((int *)writer_no)),shared_variable);

   /*EXIT SECTION*/
    semaphore_signal(&in);             //Release the semaphore and allow other readers and writers to continue.

}
```
The core idea is that writers tell the readers their necessity to access the critical section in such a way that no new readers can start working.
- Once a writer arrives, it takes the **in** semaphore and forces the subsequent readers or writers to wait.The writer releases this only once it has finished its execution.
- If there are no readers in the critical section, it executes critical section and releases the **in** semaphore upon exit.
- If there are readers that arrived before the writer,then they are allowed to execute and writer waits over **writerAccess** and sets the boolean **writer_wait** to true. 
- The last reader upon leaving the critical section,looks for waiting writers and signals or releases **writerAccess** semaphore granting access to the waiting writer.

Hence all the writers have a BOUNDED WAIT. Thus starvation is not possible with the use of a FIFO semaphore as the incoming readers that come after the writer wait in the queue in a FIFO manner. 

## Steps To Run
To run the code, ensure gcc (C-compiler) is installed in your system.
- Step 1:- Clone the repository and move into the cloned repo.
```console
    $ git clone https://github.com/pragyad3188/Starve-free-RW.git
``` 
- Step 2:- To run the first solution of starve free problem,run the command:-
```console
    $ gcc -pthread -o run starve_free_rw.c
    $ ./run 
``` 
- Step 3:- To run the optimized code of starve free problem,run the command
```console
    $ gcc -pthread -o run starve_free_rw_optimized.c
    $ ./run
```
## Conclusion
A starve free implementation of the readers writers problem is seen here.We saw two apporoaches however it is important to note that the first approach requires that the semaphore be implemented in such a way that the waiting queue is implemented as a FIFO,otherwise there may be starvation of waiting writers.

However,the second implementation(Faster Starve Free) is more generic and works without starvation even for semaphores that do not implement FIFO and wakeup threads randomly. This is due to the fact that because the probability that thread does not get a chance to execute decreases with time exponentially. And hence the waiting is bounded and not indefinite. 

Also the faster solution requires only 1 mutex lock for a reader as compared to 2 locks required for the previous case. This is a subsatntial benefit in terms of execution time.

## References
1. Operating Systems by Avi Silberschatz, Greg Gagne, and Peter Baer Galvin
2. [pthread:- Linux Man Pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)
3. [https://arxiv.org/pdf/1309.4507.pdf](https://arxiv.org/pdf/1309.4507.pdf)

