# CPUSim - A Pipelined CPU & Memory Hierarchy Simulator

## Overview
CPUSim is a CPU scheduler and memory cache simulator built in C++.
It simulates how tasks get allocated to a CPU using Round Robin
scheduling and how memory blocks move through a 3-level cache hierarchy.

## Architecture

### CPU Scheduler - Round Robin
- Each task gets CPU time in turns (quantum = 3 cycles per turn)
- If task finishes before quantum ends, next task starts immediately
- If task needs more time, it goes back to end of queue
- Fair scheduling - no task waits forever

### Memory Cache Hierarchy
Level  | Capacity  | Latency
L1     | 3 slots   | 4 cycles
L2     | 5 slots   | 12 cycles
L3     | 8 slots   | 40 cycles
RAM    | Unlimited | 200 cycles

### Cache Policy
- Search order: L1 -> L2 -> L3 -> RAM
- HIT: Block found at that level, pay that level latency
- MISS: Continue searching next level
- RAM fetch: Always succeeds, block promoted to L1
- Eviction: FIFO - oldest block removed first
- Cascade: Evicted L1 block moves to L2, evicted L2 moves to L3

## How to Compile and Run

Compile:
g++ cpusim.cpp -o cpusim

Run:
./cpusim

## Input Format
Create input.txt in same folder with this format:
TASK T1 BURST 5 MEM M1 M4 M7 TASK T2 BURST 3 MEM M2 M1 TASK T3 BURST 8 MEM M3 M6 M1 M9 TASK T4 BURST 2 MEM M4 M2

Each task has:
- ID - Task name (T1, T2 etc)
- BURST - Total CPU time needed
- MEM - Memory blocks it needs (M1, M2 etc)

## Sample Output

=== Round Robin Scheduler (Quantum=3) ===

Cycle 1 - Running: T1
    M1 -> MISS L1 -> MISS L2 -> MISS L3 -> Fetching from RAM (200 cycles)
    Cache State -> L1: [M1] L2: [] L3: []
    M4 -> MISS L1 -> MISS L2 -> MISS L3 -> Fetching from RAM (200 cycles)
    Cache State -> L1: [M1, M4] L2: [] L3: []

Cycle 604 - Running: T2
    M2 -> MISS L1 -> MISS L2 -> MISS L3 -> Fetching from RAM (200 cycles)
    Evicting M1 from L1 -> moved to L2
    Cache State -> L1: [M4, M7, M2] L2: [M1] L3: []
    M1 -> MISS L1 -> HIT in L2 (12 cycles)
    Cache State -> L1: [M7, M2, M1] L2: [M4] L3: []

==============================
=== FINAL RESULTS ===
==============================
Total Cycles      : 1582
Tasks Completed   : 4
Scheduler         : Round Robin (Quantum = 3)
Total RAM Accesses: 7
==============================

## Key Concepts Used
- Round Robin CPU Scheduling with configurable quantum
- 3 Level Cache Hierarchy with cascade eviction
- FIFO eviction policy using queue data structure
- File parsing for task input
- Pointer based task tracking for accurate remaining time

## What I Learned
- How CPU scheduling algorithms work in real operating systems
- How cache hierarchy works in real computers L1 L2 L3 RAM
- How FIFO eviction policy manages cache memory
- How memory latency affects total CPU cycles
- Cascade eviction between cache levels
- C++ implementation of queues structs pointers and file IO

## Author
Ayush Tayde
FY BTech EC - VJTI Mumbai
Project X 2026 - Global Task 2