
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <limits.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <float.h>
#include <sys/sem.h>
#include <ctype.h>
#include <sys/shm.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>
#include <mqueue.h>
#include <sys/ipc.h>
#include <sys/shm.h>



// Data structures
typedef struct {

} SetNameHere;

//For semapohre
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
} arg;

// Define a structure for shared memory data
struct Memory {
    // Define your shared data fields here
    int sharedData;
};

typedef struct {
    char name[50];
    int value;
} Constant;
