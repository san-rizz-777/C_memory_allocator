#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Testing malloc...\n");
    char *ptr = (char *)malloc(20);
    if (ptr) {
        strcpy(ptr, "Hello Allocator!");
        printf("Allocated and wrote: %s\n", ptr);
    }

    printf("\nTesting calloc...\n");
    int *arr = (int *)calloc(5, sizeof(int));
    if (arr) {
        printf("Calloc initialized array: ");
        for (int i = 0; i < 5; ++i)
            printf("%d ", arr[i]);
        printf("\n");
    }

    printf("\nTesting realloc...\n");
    ptr = (char *)realloc(ptr, 40);
    if (ptr) {
        strcat(ptr, " Extended.");
        printf("After realloc: %s\n", ptr);
    }

    printf("\nFreeing memory...\n");
    free(ptr);
    free(arr);

    return 0;
}
 /*🔧 Compile the Test with Your Allocator
Assuming you compiled your allocator to a shared object:

gcc test.c -o test
LD_PRELOAD=$PWD/memalloc.so ./test*/