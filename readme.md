## ICS-LD: a static linker for educational purpose

This is a educational static linker we use at XJTU,
in *Introduction to Computer Systems*.

### Goal

Implement the function in `src/relocation.cc` and `src/resolve.cc` to
finish relocation and symbol resolution.

### Build and Run

To build your implementation into a static linker:

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd ..
make
```

To test your implementation:

```
python3 autograder.py
```

### Instructions

Currently, instructions are under construction, and the first draft will not
be available in public (and it will be in Chinese).
When we finish tuning the project and instructions,
an English version will be posted here for those not enrolled in this course.
So please stay tuned :)

### TODO
- [ ] Add debug mode in autograder
- [ ] Test case for common block