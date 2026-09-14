#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_P 10
#define MAX_R 10

int adj[MAX_P][MAX_P];
int visited[MAX_P];
int recStack[MAX_P];
int parent[MAX_P];
int cycle_start = -1;
int cycle_end = -1;

// Construct Wait-For Graph (WFG)
void build_wait_for_graph(int n_proc, int n_res, int alloc[MAX_P][MAX_R], int req[MAX_P][MAX_R]) {
    // Initialize adjacency matrix to 0
    for (int i = 0; i < n_proc; i++) {
        for (int j = 0; j < n_proc; j++) {
            adj[i][j] = 0;
        }
    }

    // Populate edges: P_i -> P_j if P_i requests a resource currently held by P_j
    for (int i = 0; i < n_proc; i++) {
        for (int k = 0; k < n_res; k++) {
            if (req[i][k] > 0) {
                for (int j = 0; j < n_proc; j++) {
                    if (i != j && alloc[j][k] > 0) {
                        adj[i][j] = 1;
                    }
                }
            }
        }
    }
}

// Print Wait-For Graph
void print_graph(int n_proc) {
    printf(1, "Constructed Wait-For Graph (Edges P_i -> P_j):\n");
    int has_edges = 0;
    for (int i = 0; i < n_proc; i++) {
        for (int j = 0; j < n_proc; j++) {
            if (adj[i][j]) {
                printf(1, "  P%d -> P%d\n", i, j);
                has_edges = 1;
            }
        }
    }
    if (!has_edges) {
        printf(1, "  (No wait edges detected)\n");
    }
}

// DFS-based Cycle Detection
int dfs(int u, int n_proc) {
    visited[u] = 1;
    recStack[u] = 1;

    for (int v = 0; v < n_proc; v++) {
        if (adj[u][v]) {
            if (!visited[v]) {
                parent[v] = u;
                if (dfs(v, n_proc)) {
                    return 1;
                }
            } else if (recStack[v]) {
                // Back-edge found -> Cycle detected
                cycle_end = u;
                cycle_start = v;
                return 1;
            }
        }
    }

    recStack[u] = 0;
    return 0;
}

// Top-level Detection Routine
void detect_deadlock(int n_proc) {
    for (int i = 0; i < n_proc; i++) {
        visited[i] = 0;
        recStack[i] = 0;
        parent[i] = -1;
    }
    cycle_start = -1;
    cycle_end = -1;

    int deadlock_found = 0;
    for (int i = 0; i < n_proc; i++) {
        if (!visited[i]) {
            if (dfs(i, n_proc)) {
                deadlock_found = 1;
                break;
            }
        }
    }

    if (deadlock_found) {
        printf(1, "RESULT: Deadlock Detected!\n");
        printf(1, "Deadlock Cycle: ");
        
        int path[MAX_P];
        int path_len = 0;
        int curr = cycle_end;
        
        path[path_len++] = curr;
        while (curr != cycle_start && curr != -1) {
            curr = parent[curr];
            path[path_len++] = curr;
        }

        // Print from start to end to show the forward progression of the cycle
        for (int i = path_len - 1; i >= 0; i--) {
            printf(1, "P%d -> ", path[i]);
        }
        printf(1, "P%d\n", cycle_start);
    } else {
        printf(1, "RESULT: No Deadlock Detected (Graph is Acyclic).\n");
    }
}

void run_test(char *title, int n_proc, int n_res, int alloc[MAX_P][MAX_R], int req[MAX_P][MAX_R]) {
    printf(1, "\n--- %s ---\n", title);
    build_wait_for_graph(n_proc, n_res, alloc, req);
    print_graph(n_proc);
    detect_deadlock(n_proc);
}

int main(int argc, char *argv[]) {
    // Scenario 1: Acyclic Graph (No Deadlock)
    // P0 holds R0, wants R1
    // P1 holds R1, wants R2
    // P2 holds R2, wants nothing
    int alloc1[MAX_P][MAX_R] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    int req1[MAX_P][MAX_R] = {
        {0, 1, 0},
        {0, 0, 1},
        {0, 0, 0}
    };

    run_test("Scenario 1: Acyclic Wait-For Graph", 3, 3, alloc1, req1);

    // Scenario 2: 3-Process Circular Deadlock
    // P0 holds R0, wants R1
    // P1 holds R1, wants R2
    // P2 holds R2, wants R0
    int alloc2[MAX_P][MAX_R] = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };
    int req2[MAX_P][MAX_R] = {
        {0, 1, 0}, // P0 waits on P1
        {0, 0, 1}, // P1 waits on P2
        {1, 0, 0}  // P2 waits on P0
    };

    run_test("Scenario 2: 3-Process Circular Deadlock", 3, 3, alloc2, req2);

    exit();
}
