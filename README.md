# Dynamic Memory Allocator (Heap Management System)

This project is a simulation of various memory allocation strategies in C, including:

- First-Fit
- Best-Fit
- Next-Fit
- Buddy System Allocation
- Paging-based Allocation
- Smart Allocation (Automatic strategy selection)
- Memory Deallocation
- Memory Compaction
- Fragmentation Analysis (Internal & External)
- Efficiency Tracking

## 🔧 Compilation

```bash
make
```

## 🚀 Usage

```bash
./allocator
```

Then follow the prompts to allocate, deallocate, and analyze memory.

## 🧠 Features

- Tracks average steps and time taken for each allocation method.
- Automatically selects the best-fit strategy based on request size.
- Supports deallocation and memory compaction.
- Analyzes both internal and external fragmentation.

## 📂 File Descriptions

- `final.c` — Main source file containing all logic.
- `Makefile` — Easy compilation tool.

## 📝 License

This project is licensed under the MIT License.
