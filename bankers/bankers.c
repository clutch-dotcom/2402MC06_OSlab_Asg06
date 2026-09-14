#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_P 10
#define MAX_R 10

int P = 5; // Number of processes
int R = 3; // Number of resources

int Available[MAX_R];
int Max[MAX_P][MAX_R];
int Allocation[MAX_P][MAX_R];
int Need[MAX_P][MAX_R];

// Helper to compute Need = Max - Allocation
void calculate_need() {
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            Need[i][j] = Max[i][j] - Allocation[i][j];
        }
    }
}

// Safety Algorithm
int is_safe(int safe_seq[]) {
    int Work[MAX_R];
    int Finish[MAX_P];

    for (int i = 0; i < R; i++) {
        Work[i] = Available[i];
    }
    for (int i = 0; i < P; i++) {
        Finish[i] = 0;
    }

    int count = 0;
    while (count < P) {
        int found = 0;
        for (int p = 0; p < P; p++) {
            if (Finish[p] == 0) {
                int j;
                for (j = 0; j < R; j++) {
                    if (Need[p][j] > Work[j])
                        break;
                }

                if (j == R) { // Process p's needs can be met
                    for (int k = 0; k < R; k++) {
                        Work[k] += Allocation[p][k];
                    }
                    safe_seq[count++] = p;
                    Finish[p] = 1;
                    found = 1;
                }
            }
        }

        if (found == 0) {
            return 0; // System is in an unsafe state
        }
    }
    return 1; // System is safe
}

// Resource Request Algorithm
void request_resources(int process_id, int request[]) {
    printf(1, "\n--- Process P%d requesting [%d, %d, %d] ---\n", 
           process_id, request[0], request[1], request[2]);

    // Step 1: Check Request <= Need
    for (int i = 0; i < R; i++) {
        if (request[i] > Need[process_id][i]) {
            printf(1, "Error: Process P%d exceeded its maximum claim.\n", process_id);
            return;
        }
    }

    // Step 2: Check Request <= Available
    for (int i = 0; i < R; i++) {
        if (request[i] > Available[i]) {
            printf(1, "Process P%d must wait: Resources not available.\n", process_id);
            return;
        }
    }

    // Step 3: mock allocate resources
    for (int i = 0; i < R; i++) {
        Available[i] -= request[i];
        Allocation[process_id][i] += request[i];
        Need[process_id][i] -= request[i];
    }

    // Step 4: Run Safety Algorithm on tentative allocation
    int safe_seq[MAX_P];
    if (is_safe(safe_seq)) {
        printf(1, "Request GRANTED. System remains in a safe state.\n");
        printf(1, "New Safe Sequence: ");
        for (int i = 0; i < P; i++) {
            printf(1, "P%d ", safe_seq[i]);
        }
        printf(1, "\n");
    } else {
        printf(1, "Request DENIED — would lead to unsafe state.\n");
        // Rollback state
        for (int i = 0; i < R; i++) {
            Available[i] += request[i];
            Allocation[process_id][i] -= request[i];
            Need[process_id][i] += request[i];
        }
    }
}

int main(int argc, char *argv[]) {
    // Initial Classic Textbook Setup (5 processes, 3 resource types A, B, C)
    int init_Allocation[5][3] = {
        {0, 1, 0}, // P0
        {2, 0, 0}, // P1
        {3, 0, 2}, // P2
        {2, 1, 1}, // P3
        {0, 0, 2}  // P4
    };

    int init_Max[5][3] = {
        {7, 5, 3}, // P0
        {3, 2, 2}, // P1
        {9, 0, 2}, // P2
        {2, 2, 2}, // P3
        {4, 3, 3}  // P4
    };

    int init_Available[3] = {3, 3, 2};

    // Copy to global arrays
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            Allocation[i][j] = init_Allocation[i][j];
            Max[i][j] = init_Max[i][j];
        }
    }
    for (int j = 0; j < R; j++) {
        Available[j] = init_Available[j];
    }

    calculate_need();

    printf(1, "Initial Safety Check\n");
    int safe_seq[MAX_P];
    if (is_safe(safe_seq)) {
        printf(1, "System is in a SAFE state.\nInitial Safe Sequence: ");
        for (int i = 0; i < P; i++) {
            printf(1, "P%d ", safe_seq[i]);
        }
        printf(1, "\n");
    } else {
        printf(1, "System is in an UNSAFE state.\n");
        exit();
    }

    // --- Scenario 1: Valid Request (Granted) ---
    // P1 requests [1, 0, 2]
    int req1[3] = {1, 0, 2};
    request_resources(1, req1);

    // --- Scenario 2: Invalid Request (Denied - Unsafe State) ---
    // P4 requests [3, 3, 0]
    int req2[3] = {0, 2, 0};
    request_resources(4, req2);

    exit();
}