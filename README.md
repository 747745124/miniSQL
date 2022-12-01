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

## Sample Supported Query
```mysql
INSERT INTO table_name
VALUES (value1, value2, value3, ...);

CREATE TABLE table_name (
    column1 datatype,
    column2 datatype,
    column3 datatype,
   ....
);

SELECT column1, column2, ...
FROM table_name;
```
