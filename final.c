#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_MEMORY 1024
#define PAGE_SIZE 64
#define MIN_BUDDY_SIZE 32

typedef struct Block {
    int size;
    int is_free;
    int start;
    struct Block *next;
} Block;

Block *head = NULL;
Block *last_allocated = NULL;

// Efficiency tracking
int steps_first = 0, steps_best = 0, steps_next = 0;
double time_first = 0, time_best = 0, time_next = 0;
int count_first = 0, count_best = 0, count_next = 0;

void initialize_memory() {
    head = (Block*)malloc(sizeof(Block));
    head->size = MAX_MEMORY;
    head->is_free = 1;
    head->start = 0;
    head->next = NULL;
}

void display_memory() {
    Block *temp = head;
    printf("\nMemory Blocks:\n");
    while (temp) {
        printf("Start: %d, Size: %d, Free: %s\n", temp->start, temp->size, temp->is_free ? "Yes" : "No");
        temp = temp->next;
    }
}

int is_power_of_two(int n) {
    return (n > 0 && (n & (n - 1)) == 0);
}

void first_fit(int size) {
    clock_t t1 = clock();
    int steps = 0;
    Block *temp = head;
    while (temp) {
        steps++;
        if (temp->is_free && temp->size >= size) {
            if (temp->size > size) {
                Block *new_block = (Block*)malloc(sizeof(Block));
                new_block->size = temp->size - size;
                new_block->is_free = 1;
                new_block->start = temp->start + size;
                new_block->next = temp->next;
                temp->size = size;
                temp->next = new_block;
            }
            temp->is_free = 0;
            printf("Allocated %d using First-Fit (Steps: %d)\n", size, steps);
            break;
        }
        temp = temp->next;
    }
    if (!temp) printf("First-Fit Allocation failed: No suitable block.\n");
    clock_t t2 = clock();
    time_first += (double)(t2 - t1) / CLOCKS_PER_SEC;
    steps_first += steps;
    count_first++;
}

void best_fit(int size) {
    clock_t t1 = clock();
    int steps = 0;
    Block *temp = head, *best = NULL;
    while (temp) {
        steps++;
        if (temp->is_free && temp->size >= size) {
            if (!best || temp->size < best->size)
                best = temp;
        }
        temp = temp->next;
    }
    if (best) {
        if (best->size > size) {
            Block *new_block = (Block*)malloc(sizeof(Block));
            new_block->size = best->size - size;
            new_block->is_free = 1;
            new_block->start = best->start + size;
            new_block->next = best->next;
            best->size = size;
            best->next = new_block;
        }
        best->is_free = 0;
        printf("Allocated %d using Best-Fit (Steps: %d)\n", size, steps);
    } else {
        printf("Best-Fit Allocation failed: No suitable block.\n");
    }
    clock_t t2 = clock();
    time_best += (double)(t2 - t1) / CLOCKS_PER_SEC;
    steps_best += steps;
    count_best++;
}

void next_fit(int size) {
    clock_t t1 = clock();
    int steps = 0;

    if (!last_allocated) last_allocated = head;

    Block *start = last_allocated;
    Block *temp = last_allocated;

    do {
        steps++;
        if (temp->is_free && temp->size >= size) {
            if (temp->size > size) {
                Block *new_block = (Block*)malloc(sizeof(Block));
                new_block->size = temp->size - size;
                new_block->is_free = 1;
                new_block->start = temp->start + size;
                new_block->next = temp->next;
                temp->size = size;
                temp->next = new_block;
            }
            temp->is_free = 0;
            last_allocated = temp->next ? temp->next : head;
            printf("Allocated %d using Next-Fit (Steps: %d)\n", size, steps);
            clock_t t2 = clock();
            time_next += (double)(t2 - t1) / CLOCKS_PER_SEC;
            steps_next += steps;
            count_next++;
            return;
        }
        temp = temp->next ? temp->next : head;
    } while (temp != start);

    printf("Next-Fit Allocation failed.\n");
    clock_t t2 = clock();
    time_next += (double)(t2 - t1) / CLOCKS_PER_SEC;
    steps_next += steps;
    count_next++;
}

void split_block(Block *block, int size) {
    if (block->size / 2 >= size && block->size / 2 >= MIN_BUDDY_SIZE) {
        Block *buddy = (Block *)malloc(sizeof(Block));
        buddy->size = block->size / 2;
        buddy->is_free = 1;
        buddy->start = block->start + buddy->size;
        buddy->next = block->next;
        block->size /= 2;
        block->next = buddy;
        split_block(block, size);
    }
}

void buddy_allocate(int size) {
    int block_size = MIN_BUDDY_SIZE;
    while (block_size < size) block_size *= 2;
    Block *temp = head;
    while (temp) {
        if (temp->is_free && temp->size == block_size) {
            temp->is_free = 0;
            printf("Buddy Allocated block size %d at %d\n", block_size, temp->start);
            return;
        }
        if (temp->is_free && temp->size > block_size) {
            split_block(temp, block_size);
            buddy_allocate(size);
            return;
        }
        temp = temp->next;
    }
    printf("Buddy Allocation failed\n");
}

void paging_allocate(int size) {
    int pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (int i = 0; i < pages; i++) {
        first_fit(PAGE_SIZE);
    }
    printf("Paging Allocation Done for %d bytes (%d pages)\n", size, pages);
}

void deallocate(int start) {
    Block *temp = head, *prev = NULL;
    while (temp) {
        if (temp->start == start) {
            temp->is_free = 1;
            if (temp->next && temp->next->is_free) {
                Block *next = temp->next;
                temp->size += next->size;
                temp->next = next->next;
                free(next);
            }
            if (prev && prev->is_free) {
                prev->size += temp->size;
                prev->next = temp->next;
                free(temp);
            }
            printf("Deallocated block at start %d\n", start);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    printf("Deallocation failed: Invalid start address.\n");
}

void compact_memory() {
    Block *temp = head, *prev = NULL;
    int current_start = 0;
    while (temp) {
        if (!temp->is_free) {
            if (temp->start != current_start) {
                temp->start = current_start;
            }
            current_start += temp->size;
            prev = temp;
        }
        temp = temp->next;
    }

    Block *free_block = (Block *)malloc(sizeof(Block));
    free_block->start = current_start;
    free_block->size = MAX_MEMORY - current_start;
    free_block->is_free = 1;
    free_block->next = NULL;

    if (prev) prev->next = free_block;
    else head = free_block;

    printf("Memory Compacted\n");
}

void analyze_fragmentation() {
    int external = 0, internal = 0;
    Block *temp = head;
    while (temp) {
        if (temp->is_free)
            external += temp->size;
        else
            internal += (temp->size % PAGE_SIZE);
        temp = temp->next;
    }
    printf("External Fragmentation: %d, Internal Fragmentation: %d\n", external, internal);
}

void show_efficiency() {
    printf("\n--- Efficiency Summary ---\n");
    if (count_first)
        printf("First-Fit: Avg Steps: %.2f, Avg Time: %.6fs\n", (float)steps_first / count_first, time_first / count_first);
    if (count_best)
        printf("Best-Fit:  Avg Steps: %.2f, Avg Time: %.6fs\n", (float)steps_best / count_best, time_best / count_best);
    if (count_next)
        printf("Next-Fit:  Avg Steps: %.2f, Avg Time: %.6fs\n", (float)steps_next / count_next, time_next / count_next);
}

void smart_allocate(int size) {
    if (is_power_of_two(size)) buddy_allocate(size);
    else if (size % PAGE_SIZE == 0) paging_allocate(size);
    else if (size < 64) best_fit(size);
    else if (size <= 128) first_fit(size);
    else next_fit(size);
}

int main() {
    initialize_memory();

    int allocations;
    printf("Enter number of allocations: ");
    scanf("%d", &allocations);

    for (int i = 0; i < allocations; i++) {
        int size;
        printf("Enter size for allocation %d: ", i + 1);
        scanf("%d", &size);
        smart_allocate(size);
        display_memory();
    }

    int choice;
    do {
        printf("\nMenu:\n");
        printf("1. Display Memory\n2. Analyze Fragmentation\n3. Deallocate Block\n4. Compact Memory\n5. Show Efficiency\n0. Exit\nEnter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: display_memory(); break;
            case 2: analyze_fragmentation(); break;
            case 3: {
                int start;
                printf("Enter start address to deallocate: ");
                scanf("%d", &start);
                deallocate(start);
                break;
            }
            case 4: compact_memory(); break;
            case 5: show_efficiency(); break;
            case 0: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 0);

    return 0;
}
