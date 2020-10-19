# Benchmarks

The following benchmarks were performed on an _Intel Core i9-7980XE_
with GCC 9.3.

These benchmarks are not exhaustive. It can be seen that the maps have
different performance characteristics depending on usage scenario.
Benchmarks show the implementation is competitive with `tsl::robin_map`,
which is probably the fastest C++ hashtable in existence, and is
consistently faster than `google::dense_hash_map` and `absl::flat_hash_map`.

There are many cases where `zedland::hashmap` is the fastest map.

#### update 10M mod n in {1023, 16383, 65535, 1048575} records

Simple benchmark intended to show the minimum latency for small maps
with relatively constant set of keys, such as use for histograms.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`std::unordered_map::operator[]`        |    1023|    10000000|     7.3|
|`std::unordered_map::operator[]`        |   16383|    10000000|     6.5|
|`std::unordered_map::operator[]`        |   65535|    10000000|     7.3|
|`std::unordered_map::operator[]`        | 1048575|    10000000|    11.5|
|-                                       |       -|           -|       -|
|`tsl::robin_map::operator[]`            |    1023|    10000000|     1.5|
|`tsl::robin_map::operator[]`            |   16383|    10000000|     1.1|
|`tsl::robin_map::operator[]`            |   65535|    10000000|     1.1|
|`tsl::robin_map::operator[]`            | 1048575|    10000000|     4.4|
|-                                       |       -|           -|       -|
|`zedland::hashmap::operator[]`          |    1023|    10000000|     2.1|
|`zedland::hashmap::operator[]`          |   16383|    10000000|     2.1|
|`zedland::hashmap::operator[]`          |   65535|    10000000|     2.2|
|`zedland::hashmap::operator[]`          | 1048575|    10000000|     3.8|
|-                                       |       -|           -|       -|
|`zedland::linkedhashmap::operator[]`    |    1023|    10000000|     2.2|
|`zedland::linkedhashmap::operator[]`    |   16383|    10000000|     2.2|
|`zedland::linkedhashmap::operator[]`    |   65535|    10000000|     2.3|
|`zedland::linkedhashmap::operator[]`    | 1048575|    10000000|     4.7|
|-                                       |       -|           -|       -|
|`google::dense_hash_map::operator[]`    |    1023|    10000000|     3.1|
|`google::dense_hash_map::operator[]`    |   16383|    10000000|     2.8|
|`google::dense_hash_map::operator[]`    |   65535|    10000000|     2.9|
|`google::dense_hash_map::operator[]`    | 1048575|    10000000|     4.7|
|-                                       |       -|           -|       -|
|`absl::flat_hash_map::operator[]`       |    1023|    10000000|     3.4|
|`absl::flat_hash_map::operator[]`       |   16383|    10000000|     4.3|
|`absl::flat_hash_map::operator[]`       |   65535|    10000000|     6.7|
|`absl::flat_hash_map::operator[]`       | 1048575|    10000000|    35.2|

#### 10M records - insert, clear, insert, erase, insert, lookup

The first insert operation includes resize and rehash. The subsequent
insert operations shows insertion speed when the table has expanded.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`std::unordered_map::insert`            |  random|    10000000|   325.7|
|`std::unordered_map::clear`             |  random|    10000000|    61.4|
|`std::unordered_map::insert`            |  random|    10000000|   247.4|
|`std::unordered_map::lookup`            |  random|    10000000|    84.0|
|`std::unordered_map::erase`             |  random|    10000000|   272.5|
|-                                       |       -|           -|       -|
|`tsl::robin_map::insert`                |  random|    10000000|    91.0|
|`tsl::robin_map::clear`                 |  random|    10000000|    10.3|
|`tsl::robin_map::insert`                |  random|    10000000|    27.6|
|`tsl::robin_map::lookup`                |  random|    10000000|    19.9|
|`tsl::robin_map::erase`                 |  random|    10000000|    26.8|
|-                                       |       -|           -|       -|
|`zedland::hashmap::insert`              |  random|    10000000|    73.5|
|`zedland::hashmap::clear`               |  random|    10000000|     2.2|
|`zedland::hashmap::insert`              |  random|    10000000|    25.7|
|`zedland::hashmap::lookup`              |  random|    10000000|    27.7|
|`zedland::hashmap::erase`               |  random|    10000000|    28.7|
|-                                       |       -|           -|       -|
|`zedland::linkedhashmap::insert`        |  random|    10000000|   268.3|
|`zedland::linkedhashmap::clear`         |  random|    10000000|     3.3|
|`zedland::linkedhashmap::insert`        |  random|    10000000|    27.0|
|`zedland::linkedhashmap::lookup`        |  random|    10000000|    25.9|
|`zedland::linkedhashmap::erase`         |  random|    10000000|   109.5|
|-                                       |       -|           -|       -|
|`google::dense_hash_map::insert`        |  random|    10000000|    85.5|
|`google::dense_hash_map::clear`         |  random|    10000000|     1.0|
|`google::dense_hash_map::insert`        |  random|    10000000|    84.9|
|`google::dense_hash_map::lookup`        |  random|    10000000|    24.4|
|`google::dense_hash_map::erase`         |  random|    10000000|    38.1|
|-                                       |       -|           -|       -|
|`absl::flat_hash_map::insert`           |  random|    10000000|    75.2|
|`absl::flat_hash_map::clear`            |  random|    10000000|     0.7|
|`absl::flat_hash_map::insert`           |  random|    10000000|    71.6|
|`absl::flat_hash_map::lookup`           |  random|    10000000|    33.6|
|`absl::flat_hash_map::erase`            |  random|    10000000|   106.9|

#### update 1M mod n in {1023, 16383, 65535, 1048575} records

Simple benchmark intended to show the minimum latency for small maps
with relatively constant set of keys, such as use for histograms.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`std::unordered_map::operator[]`        |    1023|     1000000|    16.1|
|`std::unordered_map::operator[]`        |   16383|     1000000|     7.2|
|`std::unordered_map::operator[]`        |   65535|     1000000|     9.5|
|`std::unordered_map::operator[]`        | 1048575|     1000000|    56.6|
|-                                       |       -|           -|       -|
|`tsl::robin_map::operator[]`            |    1023|     1000000|     6.5|
|`tsl::robin_map::operator[]`            |   16383|     1000000|     1.2|
|`tsl::robin_map::operator[]`            |   65535|     1000000|     1.8|
|`tsl::robin_map::operator[]`            | 1048575|     1000000|    31.8|
|-                                       |       -|           -|       -|
|`zedland::hashmap::operator[]`          |    1023|     1000000|     2.3|
|`zedland::hashmap::operator[]`          |   16383|     1000000|     2.3|
|`zedland::hashmap::operator[]`          |   65535|     1000000|     2.8|
|`zedland::hashmap::operator[]`          | 1048575|     1000000|    18.0|
|-                                       |       -|           -|       -|
|`zedland::linkedhashmap::operator[]`    |    1023|     1000000|     2.2|
|`zedland::linkedhashmap::operator[]`    |   16383|     1000000|     2.5|
|`zedland::linkedhashmap::operator[]`    |   65535|     1000000|     3.1|
|`zedland::linkedhashmap::operator[]`    | 1048575|     1000000|    23.3|
|-                                       |       -|           -|       -|
|`google::dense_hash_map::operator[]`    |    1023|     1000000|     3.4|
|`google::dense_hash_map::operator[]`    |   16383|     1000000|     3.2|
|`google::dense_hash_map::operator[]`    |   65535|     1000000|     3.5|
|`google::dense_hash_map::operator[]`    | 1048575|     1000000|    21.2|
|-                                       |       -|           -|       -|
|`absl::flat_hash_map::operator[]`       |    1023|     1000000|     3.4|
|`absl::flat_hash_map::operator[]`       |   16383|     1000000|     4.9|
|`absl::flat_hash_map::operator[]`       |   65535|     1000000|     9.2|
|`absl::flat_hash_map::operator[]`       | 1048575|     1000000|    62.8|
|-                                       |       -|           -|       -|

#### 1M records - insert, clear, insert, erase, insert, lookup

The first insert operation includes resize and rehash. The subsequent
insert operations shows insertion speed when the table has expanded.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`std::unordered_map::insert`            |  random|     1000000|   168.6|
|`std::unordered_map::clear`             |  random|     1000000|    36.0|
|`std::unordered_map::insert`            |  random|     1000000|   134.6|
|`std::unordered_map::lookup`            |  random|     1000000|    53.7|
|`std::unordered_map::erase`             |  random|     1000000|   169.2|
|-                                       |       -|           -|       -|
|`tsl::robin_map::insert`                |  random|     1000000|    55.3|
|`tsl::robin_map::clear`                 |  random|     1000000|     7.6|
|`tsl::robin_map::insert`                |  random|     1000000|    20.3|
|`tsl::robin_map::lookup`                |  random|     1000000|    16.2|
|`tsl::robin_map::erase`                 |  random|     1000000|    21.4|
|-                                       |       -|           -|       -|
|`zedland::hashmap::insert`              |  random|     1000000|    42.3|
|`zedland::hashmap::clear`               |  random|     1000000|     1.2|
|`zedland::hashmap::insert`              |  random|     1000000|    17.7|
|`zedland::hashmap::lookup`              |  random|     1000000|    19.0|
|`zedland::hashmap::erase`               |  random|     1000000|    18.0|
|-                                       |       -|           -|       -|
|`zedland::linkedhashmap::insert`        |  random|     1000000|    96.3|
|`zedland::linkedhashmap::clear`         |  random|     1000000|     1.9|
|`zedland::linkedhashmap::insert`        |  random|     1000000|    20.3|
|`zedland::linkedhashmap::lookup`        |  random|     1000000|    21.0|
|`zedland::linkedhashmap::erase`         |  random|     1000000|    69.5|
|-                                       |       -|           -|       -|
|`google::dense_hash_map::insert`        |  random|     1000000|    51.2|
|`google::dense_hash_map::clear`         |  random|     1000000|     0.9|
|`google::dense_hash_map::insert`        |  random|     1000000|    50.9|
|`google::dense_hash_map::lookup`        |  random|     1000000|    18.4|
|`google::dense_hash_map::erase`         |  random|     1000000|    25.1|
|-                                       |       -|           -|       -|
|`absl::flat_hash_map::insert`           |  random|     1000000|    60.8|
|`absl::flat_hash_map::clear`            |  random|     1000000|     1.1|
|`absl::flat_hash_map::insert`           |  random|     1000000|    60.2|
|`absl::flat_hash_map::lookup`           |  random|     1000000|    21.7|
|`absl::flat_hash_map::erase`            |  random|     1000000|    35.5|
