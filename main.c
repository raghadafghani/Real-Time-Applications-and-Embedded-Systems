#include <stdio.h>
#include "head.h"
#include "constants.h"


#define SHM_SIZE 1024 // Define the size of the shared memory segment

int shm_id; // Define a global variable to hold the shared memory ID

pthread_mutex_t lock;
int shared_resource = 0; // Shared resource


//--------------------------------------Reading argument.txt---------------------------------------------------------

#define ARG_LENGTH 100
#define MAX_ARGS 20

void ReadArgs(int arguments[], const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open file\n");
        return;
    }

    char line[ARG_LENGTH];
    int argNum = 0;

    while (fgets(line, sizeof(line), fp) != NULL && argNum < MAX_ARGS) {
        // Remove leading and trailing whitespace
        char *trimmedLine = strtok(line, "\n\r\t");
        char *key = strtok(trimmedLine, "=");

        if (key != NULL) {
            char *valueStr = strtok(NULL, "=");
            if (valueStr != NULL) {
                int value = atoi(valueStr);
                arguments[argNum] = value;
                printf("Argument %d: %d\n", argNum, arguments[argNum]);
                argNum++;
            }
        }
    }

    fclose(fp);
}
    //===========================================
// Function to perform semaphore (acquire) operation
void acquire_semaphore(int sem_id) {
    struct sembuf acquire = {0, -1, SEM_UNDO};
    if (semop(sem_id, &acquire, 1) == -1) {
        perror("semop -- acquire");
        exit(EXIT_FAILURE);
    }
}

// Function to perform semaphore (release) operation (release)
void release_semaphore(int sem_id) {
    struct sembuf release = {0, 1, SEM_UNDO};
    if (semop(sem_id, &release, 1) == -1) {
        perror("semop -- release");
        exit(EXIT_FAILURE);
    }
}

// Signal handler function
void signal1_handler(int sig) {
    printf("SIGUSR1 received: Performing related actions.\n");
    // Additional logic can be added here
}


void signal2_handler(int sig) {
    printf("SIGUSR2 received: Performing related actions.\n");
    // Implement your logic for SIGUSR2 here
}

// This is the function that will be executed by the thread.
void* thread(void *arg)
{

    pthread_mutex_lock(&lock);
    // Critical section
    printf("Thread %ld is running...\n", (long)arg);
    shared_resource++;  // Modify shared resource
    printf("Shared Resource value: %d\n", shared_resource);
    pthread_mutex_unlock(&lock);

    return NULL;
}
/*
// Function that creates a thread
void create_thread() {
pthread_t test_thread;
// Assuming shared_shelves is already defined and initialized
void* shared_shelves = NULL; // Placeholder for your actual shared resource

// Create the thread
if (pthread_create(&test_thread, NULL, thread, shared_shelves) != 0) {
    perror("Failed to create thread");
    exit(EXIT_FAILURE);
}

// Optionally, wait for the thread to finish
if (pthread_join(test_thread, NULL) != 0) {
    perror("Failed to join thread");
    exit(EXIT_FAILURE);
}
}
*/
void create_thread() {
    int num_threads = 5;
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, thread, (void*)(long)i) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char *argv[]) {
    int arguments[MAX_ARGS];
    const char *filename = "argument.txt"; // Reading from argument.txt

    if (argc > 1) {
        filename = argv[1];
    }

    ReadArgs(arguments, filename);

    //here I can access the values from the 'arguments' array.
    int argument0 = arguments[0];
    int argument1 = arguments[1];


    int sem_id;

    struct Memory *shared_Mem = NULL; // Initialize to NULL
//Initilaize the semaphore
    key_t sem_key = ftok("supermarket", 'B');
    sem_id = semget(sem_key, 1, IPC_CREAT | 0660);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    // Create the shared memory segment
    shm_id = shmget(IPC_PRIVATE, sizeof(struct Memory), IPC_CREAT | 0660);
    if (shm_id == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // initialize the mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Mutex init has failed");
        exit(EXIT_FAILURE);
    }



    // Set up the signal handler for SIGUSR1
    if (signal(SIGUSR1, signal1_handler) == SIG_ERR) {
        perror("Signal handler setup failed");
        exit(EXIT_FAILURE);
    }


// Correct setup for SIGUSR2
    if (signal(SIGUSR2, signal2_handler) == SIG_ERR) {
        perror("Signal handler setup failed for SIGUSR2");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        shared_Mem = (struct Memory *)shmat(shm_id, NULL, 0);
        if ((void *)shared_Mem == (void *)-1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        acquire_semaphore(sem_id);

        // Access and modify shared memory
        shared_Mem->sharedData = 42;

        printf("Child process is using the resource\n");
        sleep(2); // Simulate resource usage
        printf("Child process is done using the resource\n");

        release_semaphore(sem_id);

        if (shmdt(shared_Mem) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }
        // Notify the main process that this customer is ready to pay
        kill(getppid(), SIGUSR1);
        exit(0);
    } else { // Parent process
        wait(NULL); // Wait for the child process to finish

        acquire_semaphore(sem_id);

        shared_Mem = (struct Memory *)shmat(shm_id, NULL, 0);
        if ((void *)shared_Mem == (void *)-1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        int dataFromChild = shared_Mem->sharedData;
        printf("Parent process received data from child: %d\n", dataFromChild);

        printf("Parent process is using the resource\n");
        sleep(2); // Simulate resource usage
        printf("Parent process is done using the resource\n");

        release_semaphore(sem_id);

        // Self-send SIGUSR2 to test handler
        printf("Parent process sending SIGUSR2 to itself...\n");
        kill(getpid(), SIGUSR2);

        // Wait a bit to observe the effect
        sleep(1);

        // Parent continues...
        printf("Parent process continues after SIGUSR2...\n");



        // Access and modify shared memory
        shared_Mem->sharedData = 42;
        // Call the function that creates the thread
        create_thread();

        // Continue with the rest of the program
        printf("Main thread continues...\n");


        if (shmdt(shared_Mem) == -1) {
            perror("shmdt");
            exit(EXIT_FAILURE);
        }

        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("shmctl");
            exit(EXIT_FAILURE);
        }
        //destroy the mutex
        pthread_mutex_destroy(&lock);

    }

    return 0;
}
