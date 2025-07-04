#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

//Semaphore implemented with a FIFO waiting queue
typedef struct
{
    int value;
    pthread_mutex_t lock;   //Mutex Lock 
    pthread_cond_t wait;    //Condition Variable
} semaphore;

/*
    This function creates a sempahore dynamically with given initial value 
    returns a pointer to the semaphore
*/
semaphore *init(int intital_value)
{       
        semaphore* temp=(semaphore *)malloc(sizeof(semaphore));
        if(temp == NULL) 
        {
                return NULL;
        }
        temp->value = intital_value;
        pthread_mutex_init(&(temp->lock),NULL);
        pthread_cond_init(&(temp->wait),NULL);
        return temp;
}


/*
    Wait method of a semaphore  decrements its value 
    and puts the thread in the waiting queue of semaphore 
    if it is not the first thread. 
*/
void semaphore_wait(semaphore * sem)
{
        pthread_mutex_lock(&(sem->lock));
        sem->value--;
        if(sem->value<0)
            pthread_cond_wait(&(sem->wait),&(sem->lock));
        pthread_mutex_unlock(&(sem->lock));
}


/*
    Signal method is used to increment the semaphore value and
    wakeup a blocked thread from the waiting queue in FIFO manner
*/
void semaphore_signal(semaphore *sem)
{

        pthread_mutex_lock(&(sem->lock));
        sem->value++;
        if(sem->value <= 0)
            pthread_cond_signal(&(sem->wait));
        pthread_mutex_unlock(&(sem->lock));
}


semaphore in,out,writerAccess;
int shared_variable=0,num_readers_in=0,num_readers_out=0;
bool writer_wait=0;


/*
    Writers code; All the writers attempt to write this.
*/
void *writer(void *writer_no)
{   
   /*ENTRY SECTION*/
    semaphore_wait(&in);                //This sempahore ensures that readers wait for this writer to finish 
    semaphore_wait(&out);               //Gain Access to num_readers_out
    if(num_readers_in==num_readers_out) //Checks if there are no readers in critical section
        semaphore_signal(&out);         //If yes, enter critical section and  num_readers_out variable 
    else
    {
        writer_wait=1;                  //If no, set boolean to true 
        semaphore_signal(&out);         //Release num_readers_out variable
        semaphore_wait(&writerAccess);  //Writer then waits for the readers waiting before it to finish 
        writer_wait=0;                  //Once finished,it enters the critical section and sets wait to false.
    }

   
   /*CRITICAL SECTION*/
    shared_variable+=2;
    printf("Writer %d modified shared variable to %d\n",(*((int *)writer_no)),shared_variable);

   /*EXIT SECTION*/
    semaphore_signal(&in);             //Release the semaphore and allow other readers and writers to continue.

}

/*
    Readers Code:- All the readers are synchronized in a starve free manner 
*/
void *reader(void *reader_no)
{   
   /*ENTRY SECTION*/
    semaphore_wait(&in);                        //Wait in queue for writers and readers to finish
    num_readers_in++;                               //Increment readers going in
    semaphore_signal(&in);                      //Signal waiting readers/writers

    /*CRITICAL SECTION*/
    printf("Reader %d: read shared variable as %d\n",*((int *)reader_no),shared_variable);

    /*EXIT SECTION*/
    semaphore_wait(&out);                       //Gain Access to the readers-out variable
    num_readers_out++;
    if(writer_wait==1&&num_readers_in==num_readers_out) //Here we check whether a writer is waiting ot not and current reader is last in queue
        semaphore_signal(&writerAccess);        //If yes, then grant access to a writer waiting
    semaphore_signal(&out); 
}


/*
    Main function where we perform reads and writes which result in a synchronized output.
*/
int main()
{   

    pthread_t read[6],write[5];
    in=* init(1);
    out=*init(1);
    writerAccess=*init(0);

    int a[10] = {1,2,3,4,5,6,7,8,9,10}; 


    pthread_create(&read[0], NULL, (void *)reader, (void *)&a[0]);
    pthread_create(&read[1], NULL, (void *)reader, (void *)&a[1]);
    pthread_create(&write[0], NULL, (void *)writer, (void *)&a[0]);
    pthread_create(&write[1], NULL, (void *)writer, (void *)&a[1]);
    pthread_create(&read[2], NULL, (void *)reader, (void *)&a[2]);
    pthread_create(&write[2], NULL, (void *)writer, (void *)&a[2]);
    pthread_create(&read[3], NULL, (void *)reader, (void *)&a[3]);
    pthread_create(&read[4], NULL, (void *)reader, (void *)&a[4]);
    pthread_create(&write[3], NULL, (void *)writer, (void *)&a[3]);
    pthread_create(&read[5], NULL, (void *)reader, (void *)&a[5]);
    pthread_create(&write[4], NULL, (void *)writer, (void *)&a[4]);
  

    for(int i = 0; i < 6; i++) {
        pthread_join(read[i], NULL);
    }
    for(int i = 0; i < 5; i++) {
        pthread_join(write[i], NULL);
    }

    return 0;
    
}