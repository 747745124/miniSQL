# miniSQL
A group project of a relational database system implementing functions including insertion, deletion, indexing and non-nested standard SQL queries. Written in C++14, cross-platform compiling using CMake. B+ Tree structure is used while implementing indexing.

## Usage
```bash
mkdir build
cd build
make .
```

## Performance Test
* Inserting 100k queries takes 13s to complete.
* Inserting 10k queries takes 1s to complete.
