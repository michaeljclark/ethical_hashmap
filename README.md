# zhashmap

Fast open addressing hash table and linked hash table in C++.

## Overview

This hashmap is an open-addressing hashtable similar to
`google/dense_hash_map`, however, it uses tombstone bitmaps
to eliminate necessity for empty or deleted key sentinels.
This means the hash map could potentially be made into a
drop-in replacement for `std::map` or `std::unordered_map`.
The code supports enough of the C++ unordered associative
container requirements such that it can be substituted for
many typical use-cases.

These small maps are useful for histograms and other frequently
accessed data structures where performance is critical.

## Benchmarks

The following benchmarks were performed on an Intel Core i9-7980XE
using GCC 9.2. Note this benchmark is somewhat contrived, but is
intended to measure minimum latency for small histogram hashtables.

#### update 10M mod 16383 records

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`tsl::robin_map`                        |   16383|    10000000|     0.9|
|`zedland::hashmap::operator[]`          |   16383|    10000000|     1.4|
|`zedland::linkedhashmap::operator[]`    |   16383|    10000000|     2.0|
|`google::dense_hash_map::operator[]`    |   16383|    10000000|     2.1|
|`absl::flat_hash_map::operator[]`       |   16383|    10000000|     4.0|
|`std::unordered_map::operator[]`        |   16383|    10000000|     6.3|

#### 10M records - insert, clear, insert, erase, insert, lookup

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|std::unordered_map::insert              |  random|    10000000|   380.8|   3.808|
|std::unordered_map::clear               |  random|    10000000|    68.3|   0.683|
|std::unordered_map::insert              |  random|    10000000|   223.5|   2.235|
|std::unordered_map::erase               |  random|    10000000|   263.8|   2.638|
|std::unordered_map::insert              |  random|    10000000|   223.9|   2.239|
|std::unordered_map::lookup              |  random|    10000000|    72.9|   0.729|

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|tsl::robin_map::insert                  |  random|    10000000|    87.1|   0.871|
|tsl::robin_map::clear                   |  random|    10000000|     9.8|   0.098|
|tsl::robin_map::insert                  |  random|    10000000|    27.5|   0.275|
|tsl::robin_map::erase                   |  random|    10000000|    27.8|   0.278|
|tsl::robin_map::insert                  |  random|    10000000|    28.5|   0.285|
|tsl::robin_map::lookup                  |  random|    10000000|    19.8|   0.198|

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|zedland::hashmap::insert                |  random|    10000000|    67.3|   0.673|
|zedland::hashmap::clear                 |  random|    10000000|     2.2|   0.022|
|zedland::hashmap::insert                |  random|    10000000|    23.1|   0.231|
|zedland::hashmap::erase                 |  random|    10000000|    29.7|   0.297|
|zedland::hashmap::insert                |  random|    10000000|    23.2|   0.232|
|zedland::hashmap::lookup                |  random|    10000000|    27.4|   0.274|

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|zedland::linkedhashmap::insert          |  random|    10000000|   270.2|   2.702|
|zedland::linkedhashmap::clear           |  random|    10000000|     3.4|   0.034|
|zedland::linkedhashmap::insert          |  random|    10000000|    26.4|   0.264|
|zedland::linkedhashmap::erase           |  random|    10000000|   113.4|   1.134|
|zedland::linkedhashmap::insert          |  random|    10000000|    26.9|   0.269|
|zedland::linkedhashmap::lookup          |  random|    10000000|    26.2|   0.262|

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|google::dense_hash_map::insert          |  random|    10000000|    83.0|   0.830|
|google::dense_hash_map::clear           |  random|    10000000|     1.0|   0.010|
|google::dense_hash_map::insert          |  random|    10000000|    83.0|   0.830|
|google::dense_hash_map::erase           |  random|    10000000|    36.9|   0.369|
|google::dense_hash_map::insert          |  random|    10000000|    93.2|   0.932|
|google::dense_hash_map::lookup          |  random|    10000000|    24.6|   0.246|

|container                               |  spread|       count| time_ns|duration|
|:-------------------------------------- |  -----:|       ----:| ------:| ------:|
|absl::flat_hash_map::insert             |  random|    10000000|    70.6|   0.706|
|absl::flat_hash_map::clear              |  random|    10000000|     0.7|   0.007|
|absl::flat_hash_map::insert             |  random|    10000000|    72.2|   0.722|
|absl::flat_hash_map::erase              |  random|    10000000|   106.7|   1.067|
|absl::flat_hash_map::insert             |  random|    10000000|    55.6|   0.556|
|absl::flat_hash_map::lookup             |  random|    10000000|    33.9|   0.339|

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
