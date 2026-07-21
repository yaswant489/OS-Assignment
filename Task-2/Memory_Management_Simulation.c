
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 4      // Number of physical memory frames
#define MAX_PAGES  100     // Max reference string length

/* FIFO Page Replacement */

void fifo_replacement(int pages[], int n, int num_frames) {
    int frames[MAX_FRAMES];
    for (int i = 0; i < num_frames; i++)
        frames[i] = -1;   // -1 means frame is empty

    int front = 0;         // points to the oldest loaded page for replacement
    int page_faults = 0;
    int page_hits = 0;

    printf("\n=== FIFO Page Replacement (Frames = %d) ===\n", num_frames);

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        int found = 0;

        // Check if page already in a frame
        for (int j = 0; j < num_frames; j++) {
            if (frames[j] == page) {
                found = 1;
                break;
            }
        }

        if (found) {
            page_hits++;
            printf("Page %d -> HIT   | Frames: ", page);
        } else {
            // Page fault: load page, replace oldest
            frames[front] = page;
            front = (front + 1) % num_frames;
            page_faults++;
            printf("Page %d -> FAULT | Frames: ", page);
        }

        // Log current frame state
        for (int j = 0; j < num_frames; j++) {
            if (frames[j] == -1)
                printf("[ ] ");
            else
                printf("[%d] ", frames[j]);
        }
        printf("\n");
    }

    printf("\nFIFO Results:\n");
    printf("  Total References: %d\n", n);
    printf("  Page Faults: %d\n", page_faults);
    printf("  Page Hits: %d\n", page_hits);
    printf("  Fault Ratio: %.2f%%\n", (page_faults * 100.0) / n);
    printf("  Hit Ratio:   %.2f%%\n", (page_hits * 100.0) / n);
}

/* LRU Page Replacement */

void lru_replacement(int pages[], int n, int num_frames) {
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES]; // tracks the "time" each frame was last accessed
    for (int i = 0; i < num_frames; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    int page_faults = 0;
    int page_hits = 0;

    printf("\n=== LRU Page Replacement (Frames = %d) ===\n", num_frames);

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        int found = -1;

        // Check if page already in a frame
        for (int j = 0; j < num_frames; j++) {
            if (frames[j] == page) {
                found = j;
                break;
            }
        }

        if (found != -1) {
            page_hits++;
            last_used[found] = i;   // update recency
            printf("Page %d -> HIT   | Frames: ", page);
        } else {
            // Page fault: find frame to replace
            int replace_idx = -1;

            // First try an empty frame
            for (int j = 0; j < num_frames; j++) {
                if (frames[j] == -1) {
                    replace_idx = j;
                    break;
                }
            }

            // No empty frame: find least recently used
            if (replace_idx == -1) {
                int oldest_time = last_used[0];
                replace_idx = 0;
                for (int j = 1; j < num_frames; j++) {
                    if (last_used[j] < oldest_time) {
                        oldest_time = last_used[j];
                        replace_idx = j;
                    }
                }
            }

            frames[replace_idx] = page;
            last_used[replace_idx] = i;
            page_faults++;
            printf("Page %d -> FAULT | Frames: ", page);
        }

        for (int j = 0; j < num_frames; j++) {
            if (frames[j] == -1)
                printf("[ ] ");
            else
                printf("[%d] ", frames[j]);
        }
        printf("\n");
    }

    printf("\nLRU Results:\n");
    printf("  Total References: %d\n", n);
    printf("  Page Faults: %d\n", page_faults);
    printf("  Page Hits: %d\n", page_hits);
    printf("  Fault Ratio: %.2f%%\n", (page_faults * 100.0) / n);
    printf("  Hit Ratio:   %.2f%%\n", (page_hits * 100.0) / n);
}

/* Configurable Page Size Demo */

void demonstrate_paging(int virtual_address, int page_size) {
    int page_number = virtual_address / page_size;
    int offset = virtual_address % page_size;

    printf("Virtual Address: %d | Page Size: %d bytes\n", virtual_address, page_size);
    printf("  -> Page Number: %d\n", page_number);
    printf("  -> Offset within page: %d\n", offset);
}

/* MAIN */

int main() {
    // Reference string simulating a sequence of page accesses by a process
    int pages[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
    int n = sizeof(pages) / sizeof(pages[0]);
    int num_frames = 3; // 3 physical frames available

    printf("=== Task 2: Memory Management Simulation ===\n");
    printf("Reference String: ");
    for (int i = 0; i < n; i++) printf("%d ", pages[i]);
    printf("\n");

    // Demonstrate configurable page size / address translation
    printf("\n=== Paging: Address Translation Demo (page size = 4096 bytes) ===\n");
    int addresses[] = {0, 4100, 8195, 16000};
    for (int i = 0; i < 4; i++)
        demonstrate_paging(addresses[i], 4096);

    // Run both replacement algorithms on the same reference string
    fifo_replacement(pages, n, num_frames);
    lru_replacement(pages, n, num_frames);

    // Comparison summary
    printf("\n=== Comparison Summary ===\n");
    printf("Both algorithms were tested with %d frames on the same reference string.\n", num_frames);
    printf("See individual results above to compare fault/hit ratios.\n");

    return 0;
}
