# 🧠  Memory Allocator in C

A minimal yet functional implementation of a `malloc`-like memory allocator in C.
This project demonstrates how memory management works at a low level, showing how `malloc`, `free`, `calloc`, and `realloc` can be implemented from scratch using system calls like `sbrk()`.

---

## 🏗️ How It Works

At its core, this memory allocator uses a **linked list of memory blocks**. Each block contains:

- A **header** (metadata): size, is_free flag, and a pointer to the next block
- The actual **memory region** returned to the user

This structure enables:
- Reuse of freed blocks
- Simple traversal for memory allocation
- Shrinking the heap when possible

---

## 📦 Memory Block Structure

```c
typedef union header {
    struct {
        size_t size;        // Size of the user memory
        unsigned is_free;   // 0 = used, 1 = free
        union header *next; // Pointer to the next block
    } s;
    char stub[16];          // Ensures 16-byte alignment
} header_t;
```

Each allocated block includes this header, hidden from the user.

---

## 🔩 System Call Used: `sbrk()`

The allocator expands or shrinks the program's heap using the `sbrk()` system call:

- `sbrk(n)` increases the heap by `n` bytes.
- `sbrk(0)` returns the current program break (end of the heap).
- `sbrk(-n)` releases memory back to the OS.

This system call gives the allocator full control over the heap segment.

---

## 🔐 Thread Safety

A `pthread_mutex_t` named `global_malloc_lock` ensures thread safety during memory allocation and deallocation. This prevents race conditions when multiple threads access or modify the memory blocks.



## 🧰 Implemented Functions

### `void* malloc(size_t size)`
- Allocates memory.
- Reuses free blocks if available.
- Expands the heap if needed.

### `void free(void* ptr)`
- Marks a block as free.
- If it's the last block, shrinks the heap with `sbrk(-n)`.

### `void* calloc(size_t num, size_t size)`
- Allocates and zeroes out memory.
- Detects and prevents integer overflows.

### `void* realloc(void* ptr, size_t size)`
- Changes the size of the allocated block.
- If new size fits in the old block, reuse it.
- Otherwise, allocate a new block, copy content, and free the old one.

### `void print_mem_list()`
- Debugging tool to print the current state of the memory list.

---

## 📌 Why Implement a Custom Allocator?

- 🔍 **Educational**: Understand how malloc works internally.
- 🛠️ **Control**: Gain fine-grained control over memory allocation behavior.
- ⚙️ **Customization**: Optimize for special use cases (e.g., memory pools).
- 🚀 **Performance Insight**: Learn about fragmentation, reuse, and memory reuse patterns.

---

## 🧪 Example Output (Debug)

```text
head = 0x55c4..., tail = 0x55c4...
addr = 0x55c4..., size = 32, is_free=0, next=0x0
```

---

## 🧠 Concept of Program Break

The **program break** is the end of the process's data segment. With `sbrk()`, you can move this break to allocate or release memory.

- `sbrk(0)` → current program break (top of the heap)
- `sbrk(n)` → increase program break by `n` (allocate memory)
- `sbrk(-n)` → reduce program break by `n` (deallocate memory)

> This simulates the way `malloc` interacts with the kernel’s memory management system.

---

## 🚧 Future enhancements

-  Coalescing of adjacent free blocks
-  Optimized for performance
-  Boundary tags or segregated free lists
- ⚠️ `sbrk()` is deprecated on some modern systems in favor of `mmap()`/`brk()`

---

## 🚀 How to Run (Note : Majorly works on "Linux distros and some macOS" ) 

## 🛠️ Compiling and Using the Custom Allocator

You can get the code from this repository and compile the memory allocator as a **shared object** to preload it into standard programs.

### Step 1: Compile as a Shared Library

```bash
gcc -o memalloc.so -fPIC -shared memalloc.c
```

- `-fPIC`: Generates position-independent code.
- `-shared`: Creates a shared object suitable for dynamic linking.

### Step 2: Preload with `LD_PRELOAD`

On Linux, use the `LD_PRELOAD` environment variable to load your memory allocator before any standard library:

```bash
export LD_PRELOAD=$PWD/memalloc.so
```

### Step 3: Run Any Program with Your Allocator

```bash
ls
```

You should see debug messages like:

```text
Heyyyy!!I am using malloc!!
```

This proves that your custom `malloc`, `free`, `calloc`, and `realloc` are being used instead of the system's standard library implementations.

> 🧪 Try printing debug output inside `malloc()` or `free()` to trace memory usage in any program dynamically.

---


