#include "types.h"
#include "stat.h"
#include "user.h"

// Define a struct to hold our locks and shared data in the shared memory page
struct shm_locks {
    int lock1;
    int lock2;
    int shared_resource;
    int print_mutex; // Optional: to prevent interleaved printf output
};

// Busy-wait delay to reliably trigger the race condition
void delay() {
    int dummy = 0;
    (void)dummy; // Prevent unused variable warning
    for (int i = 0; i < 20000000; i++) {
        dummy += i;
    }
}

int main(void) {
    int pid;

    // Retrieve pointer to the shared resource page
    struct shm_locks *shm = (struct shm_locks *) shmget();
    shm->shared_resource = 0;

    // ==========================================
    // SCENARIO 1: FIXED (Deadlock Prevention)
    // ==========================================
    printf(1, "\n=== RUNNING SCENARIO 1: FIXED (Global Ordering) ===\n");
    
    // Initialize binary semaphores (1 = available)
    shm->lock1 = sem_init(1);
    shm->lock2 = sem_init(1);
    shm->print_mutex = sem_init(1);

    pid = fork();
    if (pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }

    if (pid == 0) {
        // --- CHILD (Process B) ---
        // Acquires Lock 1 FIRST (Following strict global ordering)
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Attempting to acquire Lock 1...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock1);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Acquired Lock 1. Busy-waiting...\n");
        sem_post(shm->print_mutex);
        
        delay();
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Attempting to acquire Lock 2...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock2);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Acquired Lock 2! Accessing Shared Resource...\n");
        sem_post(shm->print_mutex);
        
        // Critical Section
        shm->shared_resource++; 
        
        sem_post(shm->lock2);
        sem_post(shm->lock1);
        exit(); 
    } else {
        // --- PARENT (Process A) ---
        // Acquires Lock 1 FIRST
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Attempting to acquire Lock 1...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock1);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Acquired Lock 1. Busy-waiting...\n");
        sem_post(shm->print_mutex);
        
        delay();
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Attempting to acquire Lock 2...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock2);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Acquired Lock 2! Accessing Shared Resource...\n");
        sem_post(shm->print_mutex);
        
        // Critical Section
        shm->shared_resource++; 
        
        sem_post(shm->lock2);
        sem_post(shm->lock1);
        
        wait(); // Wait for Process B to finish
        printf(1, ">>> Scenario 1 Completed. Shared Resource Value: %d\n", shm->shared_resource);
    }

    // ==========================================
    // SCENARIO 2: BAD (Deliberate Deadlock)
    // ==========================================
    printf(1, "\n=== RUNNING SCENARIO 2: BAD (Circular Wait) ===\n");
    printf(1, "(Note: The system will hang at the end of this run.)\n\n");
    
    // Re-initialize binary semaphores and shared memory
    shm->lock1 = sem_init(1);
    shm->lock2 = sem_init(1);
    shm->shared_resource = 0; 

    pid = fork();
    if (pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }

    if (pid == 0) {
        // --- CHILD (Process B) ---
        // Acquires Lock 2 FIRST (Violates ordering, causes circular wait)
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Attempting to acquire Lock 2...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock2);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Acquired Lock 2. Busy-waiting...\n");
        sem_post(shm->print_mutex);
        
        delay();
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process B] Attempting to acquire Lock 1...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock1);
        
        // The program will never reach this point
        printf(1, "[Process B] Acquired Lock 1!\n");
        shm->shared_resource++;
        
        sem_post(shm->lock1);
        sem_post(shm->lock2);
        exit();
    } else {
        // --- PARENT (Process A) ---
        // Acquires Lock 1 FIRST
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Attempting to acquire Lock 1...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock1);
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Acquired Lock 1. Busy-waiting...\n");
        sem_post(shm->print_mutex);
        
        delay();
        
        sem_wait(shm->print_mutex);
        printf(1, "[Process A] Attempting to acquire Lock 2...\n");
        sem_post(shm->print_mutex);
        
        sem_wait(shm->lock2);
        
        // The program will never reach this point
        printf(1, "[Process A] Acquired Lock 2!\n");
        shm->shared_resource++;

        sem_post(shm->lock2);
        sem_post(shm->lock1);
        
        wait();
        printf(1, ">>> Scenario 2 Completed.\n");
        exit();
    }
}