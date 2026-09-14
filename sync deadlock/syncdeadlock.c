#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCS 5
#define NUM_CYCLES 3

// Define a struct for our shared semaphores in the shared memory page
struct shm_sync {
    int sem_printer; // 2 instances
    int sem_disk;    // 2 instances
    int sem_scanner; // 1 instance
    int print_mutex; // For clean terminal output
};

// Helper to return resource name
char* get_res_name(int res_id) {
    if (res_id == 0) return "Printer";
    if (res_id == 1) return "Disk";
    if (res_id == 2) return "Scanner";
    return "Unknown";
}

// Process worker function
void do_work(int pid, int res1_id, int res2_id, int sem1, int sem2, struct shm_sync *shm) {
    for (int cycle = 1; cycle <= NUM_CYCLES; cycle++) {
        // 1. Request First Resource
        sem_wait(shm->print_mutex);
        printf(1, "Process %d (Cycle %d): Requesting %s...\n", pid, cycle, get_res_name(res1_id));
        sem_post(shm->print_mutex);
        
        sem_wait(sem1);

        // 2. Request Second Resource
        sem_wait(shm->print_mutex);
        printf(1, "Process %d (Cycle %d): Granted %s. Requesting %s...\n", pid, cycle, get_res_name(res1_id), get_res_name(res2_id));
        sem_post(shm->print_mutex);
        
        sem_wait(sem2);

        // 3. Work (Critical Section)
        sem_wait(shm->print_mutex);
        printf(1, "Process %d (Cycle %d): Granted %s. WORKING...\n", pid, cycle, get_res_name(res2_id));
        sem_post(shm->print_mutex);
        
        sleep(20); // Simulate task execution

        // 4. Release Resources 
        sem_post(sem2);
        sem_post(sem1);

        sem_wait(shm->print_mutex);
        printf(1, "Process %d (Cycle %d): Finished work. Released %s and %s.\n", pid, cycle, get_res_name(res2_id), get_res_name(res1_id));
        sem_post(shm->print_mutex);
        
        sleep(10); // Rest before next cycle
    }
}

int main(void) {
    struct shm_sync *shm = (struct shm_sync *) shmget();
    
    // Initialize counting semaphores based on available hardware instances
    shm->sem_printer = sem_init(2);
    shm->sem_disk    = sem_init(2);
    shm->sem_scanner = sem_init(1);
    shm->print_mutex = sem_init(1);

    printf(1, "--- Starting Multi-Resource Sync (Deadlock Prevention via Ordering) ---\n");
    printf(1, "Resources Available: 2 Printers, 2 Disks, 1 Scanner\n\n");

    for (int i = 0; i < NUM_PROCS; i++) {
        if (fork() == 0) {
            // Assign resource requirements to processes.
            // Resources are ALWAYS requested in ascending order of their IDs:
            // 0 (Printer) -> 1 (Disk) -> 2 (Scanner).
            if (i == 0) {
                do_work(i, 0, 1, shm->sem_printer, shm->sem_disk, shm);
            } else if (i == 1) {
                do_work(i, 1, 2, shm->sem_disk, shm->sem_scanner, shm);
            } else if (i == 2) {
                do_work(i, 0, 2, shm->sem_printer, shm->sem_scanner, shm);
            } else if (i == 3) {
                do_work(i, 0, 1, shm->sem_printer, shm->sem_disk, shm);
            } else if (i == 4) {
                do_work(i, 0, 2, shm->sem_printer, shm->sem_scanner, shm);
            }
            exit();
        }
    }

    // Wait for all 5 child processes to finish
    for (int i = 0; i < NUM_PROCS; i++) {
        wait();
    }

    printf(1, "\nAll processes completed successfully. No deadlocks occurred.\n");
    exit();
}