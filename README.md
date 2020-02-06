# zhashmap

Fast open addressing hash table in C++.

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
|zhashmap<ident>::insert       |  random|     1000000|    66.0|
|zhashmap<FNV1amc>::insert     |  random|     1000000|    80.2|
|std::unordered_map::insert    |  random|     1000000|   150.0|
|java.util.HashMap::insert     |  random|     1000000|   194.0|
|std::map::insert              |  random|     1000000|   386.2|

#### lookup 1M records

|container                     |  spread|       count| time_ns|
|:---------------------------- |  -----:|       ----:| ------:|
|zhashmap<ident>::lookup       |  random|     1000000|    17.9|
|dense_hash_map::lookup        |  random|     1000000|    21.3|
|java.util.HashMap::lookup     |  random|     1000000|    29.0|
|zhashmap<FNV1amc>::lookup     |  random|     1000000|    43.1|
|std::unordered_map::lookup    |  random|     1000000|    56.2|
|std::map::lookup              |  random|     1000000|   384.7|

#### update 10M mod 16383 records

|container                     |  spread|       count| time_ns|
|:---------------------------- |  -----:|       ----:| ------:|
|zhashmap<ident>::operator[]   |   16383|    10000000|     1.8|
|dense_hash_map::operator[]    |   16383|    10000000|     2.4|
|std::unordered_map::operator[]|   16383|    10000000|     6.4|
|zhashmap<FNV1amc>::operator[] |   16383|    10000000|     8.1|
|std::map::operator[]          |   16383|    10000000|    56.3|

## Code Analysis

The hashtable lookup code is 27 instructions of x86_64 code
when compiled with Clang 9.0:

```
find:
            mov     rax, rdi
            mov     r8, qword ptr [rsi]
            mov     rdx, qword ptr [rdi + 8]
            mov     r9, qword ptr [rdi + 24]
            lea     r10, [rdx - 1]
            mov     rsi, r10
            and     rsi, r8
            jmp     .LBB1_1
.LBB1_6:    add     rsi, 1
            and     rsi, r10
.LBB1_1:    mov     rcx, rsi
            shr     rcx, 5
            mov     rdi, qword ptr [r9 + 8*rcx]
            lea     ecx, [rsi + rsi]
            shr     rdi, cl
            and     edi, 3
            cmp     edi, 2
            je      .LBB1_6
            test    edi, edi
            je      .LBB1_5
            mov     rcx, qword ptr [rax + 16]
            mov     rdi, rsi
            shl     rdi, 4
            cmp     qword ptr [rcx + rdi], r8
            jne     .LBB1_6
            mov     rdx, rsi
.LBB1_5:    ret
```

## License

This software is released under the ISC license:

```
Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```
