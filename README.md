# Ferry Tour Simulation - Operating Systems Project

This project simulates a multithreaded ferry transportation system using POSIX threads (pthreads) in C. Vehicles must cross a city via a ferry that operates between two sides, adhering to real-world constraints such as tolls, ferry capacity, and synchronization between concurrent threads.

---

## Project Description

The system models:

•⁠  ⁠A ferry with *capacity of 20 car units*.
•⁠  ⁠*12 cars, **10 minibuses, **8 trucks* — each must complete a round trip.
•⁠  ⁠*4 toll booths* (2 on each side).
•⁠  ⁠Vehicles pass tolls one at a time and wait in a square.
•⁠  ⁠Ferry loads vehicles *one at a time*, only when conditions are met:
  - Correct vehicle side and trip stage.
  - Not overloaded.
•⁠  ⁠Ferry only departs when at least one vehicle is loaded.
•⁠  ⁠Vehicles return to their original side.

---

## Vehicle Capacity Mapping

| Vehicle Type | Size (Units) |
| ------------ | ------------ |
| Car          | 1            |
| Minibus      | 2            |
| Truck        | 3            |

---

## Technologies Used

•⁠  ⁠*Language:* C
•⁠  ⁠*Concurrency:* POSIX Threads (⁠ pthread ⁠)

---

## Threads Overview

•⁠  ⁠*30 Vehicle Threads* – Each vehicle independently:
  - Passes toll
  - Waits to board ferry
  - Travels and returns
•⁠  ⁠*1 Ferry Thread* – Handles:
  - Boarding
  - Travel simulation
  - Unloading

---

## Key Synchronization Concepts

•⁠  ⁠*Mutexes (⁠ pthread_mutex_t ⁠)* for tolls, shared data protection.
•⁠  ⁠*Condition Variables (⁠ pthread_cond_t ⁠)* for controlling ferry boarding.
•⁠  ⁠Custom logic to *prevent infinite loops* and *empty ferry trips*.

---

## Compilation and Execution

### To Compile && To Run:

⁠ bash
gcc -pthread -o ferry ferry.c

./ferry

Bu proje [Berra Söyler](https://github.com/berriesyl) ve [Dilek Çelebi](https://github.com/dilekk1) tarafından hazırlanmıştır.
