# hashmap

Fast C++ open addressing hash map.

## Overview

This hashmap is an open-addressing hashtable similar to
`google/dense_hash_map`, however, it uses tombstone bitmaps
to eliminate necessity for empty or deleted key sentinels.
This means the hash map could potentially be made into a
drop-in replacement for `std::map` or `std::unordered_map`.

These small maps are useful for histograms and other frequently
accessed data structures where performance is critical.

## Benchmarks

The following benchmarks were performed on an Intel Core i9-7980XE
using GCC 9.2:

#### insert 1M records

|container                     |  spread|       count| time_ns|
|:---------------------------- |  -----:|       ----:| ------:|
|dense_hash_map::insert        |  random|     1000000|    48.8|
|hashmap<ident>::insert        |  random|     1000000|    66.0|
|hashmap<FNV1amc>::insert      |  random|     1000000|    80.2|
|std::unordered_map::insert    |  random|     1000000|   150.0|
|java.util.HashMap::insert     |  random|     1000000|   194.0|
|std::map::insert              |  random|     1000000|   386.2|

#### lookup 1M records

|container                     |  spread|       count| time_ns|
|:---------------------------- |  -----:|       ----:| ------:|
|hashmap<ident>::lookup        |  random|     1000000|    17.9|
|dense_hash_map::lookup        |  random|     1000000|    21.3|
|java.util.HashMap::lookup     |  random|     1000000|    29.0|
|hashmap<FNV1amc>::lookup      |  random|     1000000|    43.1|
|std::unordered_map::lookup    |  random|     1000000|    56.2|
|std::map::lookup              |  random|     1000000|   384.7|

#### update 10M mod 16383 records

|container                     |  spread|       count| time_ns|
|:---------------------------- |  -----:|       ----:| ------:|
|hashmap<ident>::operator[]    |   16383|    10000000|     1.8|
|dense_hash_map::operator[]    |   16383|    10000000|     2.4|
|std::unordered_map::operator[]|   16383|    10000000|     6.4|
|hashmap<FNV1amc>::operator[]  |   16383|    10000000|     8.1|
|std::map::operator[]          |   16383|    10000000|    56.3|
