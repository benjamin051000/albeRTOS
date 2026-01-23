# albeRTOS
A simple [Real-time operating system](https://en.wikipedia.org/wiki/Real-time_operating_system) for i686 (32-bit x86).

# Features
- Thread-based parallelism
- Round-robin real-time scheduler
- Resource-sharing constructs (FIFO, Semaphore)
- Periodic events (timeout-driven)
- Aperiodic events (interrupt-driven)

---

# Usage

Use docker or podman for building the kernel.
Use qemu to run the operating system.

## Setup
- Install docker or podman.
- Install qemu

- Run `docker build -t albertos .` to build the docker image used to compile the kernel.

## Building 
- Build the kernel with `docker run -v .:/albeRTOS make -j$(nproc)`
- On Fedora, you may need to mount via `-v .:/albeRTOS:Z` for... SELinux reasons.

## Run an example
- TODO 

---

# Testing
- TODO write some tests

---
This project is originally from my implementation of the **G8RTOS**, an RTOS kernel in C created in Microprocessor Applications 2 at the University of Florida. 🐊
