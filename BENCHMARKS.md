# Benchmarks

The following benchmarks were performed on an _Intel Core i9-7980XE_
with GCC 9.3.

These benchmarks are not exhaustive. It can be seen that the maps have
different performance characteristics depending on usage. The compiler
has a significant effect on performance, and there are many scenarios
where `zedland::hashmap` is the fastest map.

#### 10M update mod n in {1023, 16383, 65535, 1048575} records

Simple benchmark showing the minimum latency for small maps with
constant spread of keys, simulating histogram use-case.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|_std::unordered_map::operator[]_        |    1023|    10000000|     7.3|
|_std::unordered_map::operator[]_        |   16383|    10000000|     6.5|
|_std::unordered_map::operator[]_        |   65535|    10000000|     7.3|
|_std::unordered_map::operator[]_        | 1048575|    10000000|    11.5|
|-                                       |       -|           -|       -|
|_tsl::robin_map::operator[]_            |    1023|    10000000|     1.5|
|_tsl::robin_map::operator[]_            |   16383|    10000000|     1.1|
|_tsl::robin_map::operator[]_            |   65535|    10000000|     1.1|
|_tsl::robin_map::operator[]_            | 1048575|    10000000|     4.4|
|-                                       |       -|           -|       -|
|_zedland::hashmap::operator[]_          |    1023|    10000000|     2.1|
|_zedland::hashmap::operator[]_          |   16383|    10000000|     2.1|
|_zedland::hashmap::operator[]_          |   65535|    10000000|     2.2|
|_zedland::hashmap::operator[]_          | 1048575|    10000000|     3.8|
|-                                       |       -|           -|       -|
|_zedland::linkedhashmap::operator[]_    |    1023|    10000000|     2.2|
|_zedland::linkedhashmap::operator[]_    |   16383|    10000000|     2.2|
|_zedland::linkedhashmap::operator[]_    |   65535|    10000000|     2.3|
|_zedland::linkedhashmap::operator[]_    | 1048575|    10000000|     4.7|
|-                                       |       -|           -|       -|
|_google::dense_hash_map::operator[]_    |    1023|    10000000|     3.1|
|_google::dense_hash_map::operator[]_    |   16383|    10000000|     2.8|
|_google::dense_hash_map::operator[]_    |   65535|    10000000|     2.9|
|_google::dense_hash_map::operator[]_    | 1048575|    10000000|     4.7|
|-                                       |       -|           -|       -|
|_absl::flat_hash_map::operator[]_       |    1023|    10000000|     3.4|
|_absl::flat_hash_map::operator[]_       |   16383|    10000000|     4.3|
|_absl::flat_hash_map::operator[]_       |   65535|    10000000|     6.7|
|_absl::flat_hash_map::operator[]_       | 1048575|    10000000|    35.2|

#### 10M records - insert, clear, insert, erase, insert, lookup

First insert operation calls resize and rehash, subsequent insert
operations shows insertion speed when table has expanded.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|_std::unordered_map::insert_            |  random|    10000000|   325.7|
|_std::unordered_map::clear_             |  random|    10000000|    61.4|
|_std::unordered_map::insert_            |  random|    10000000|   247.4|
|_std::unordered_map::lookup_            |  random|    10000000|    84.0|
|_std::unordered_map::erase_             |  random|    10000000|   272.5|
|-                                       |       -|           -|       -|
|_tsl::robin_map::insert_                |  random|    10000000|    91.0|
|_tsl::robin_map::clear_                 |  random|    10000000|    10.3|
|_tsl::robin_map::insert_                |  random|    10000000|    27.6|
|_tsl::robin_map::lookup_                |  random|    10000000|    19.9|
|_tsl::robin_map::erase_                 |  random|    10000000|    26.8|
|-                                       |       -|           -|       -|
|_zedland::hashmap::insert_              |  random|    10000000|    73.5|
|_zedland::hashmap::clear_               |  random|    10000000|     2.2|
|_zedland::hashmap::insert_              |  random|    10000000|    25.7|
|_zedland::hashmap::lookup_              |  random|    10000000|    27.7|
|_zedland::hashmap::erase_               |  random|    10000000|    28.7|
|-                                       |       -|           -|       -|
|_zedland::linkedhashmap::insert_        |  random|    10000000|   268.3|
|_zedland::linkedhashmap::clear_         |  random|    10000000|     3.3|
|_zedland::linkedhashmap::insert_        |  random|    10000000|    27.0|
|_zedland::linkedhashmap::lookup_        |  random|    10000000|    25.9|
|_zedland::linkedhashmap::erase_         |  random|    10000000|   109.5|
|-                                       |       -|           -|       -|
|_google::dense_hash_map::insert_        |  random|    10000000|    85.5|
|_google::dense_hash_map::clear_         |  random|    10000000|     1.0|
|_google::dense_hash_map::insert_        |  random|    10000000|    84.9|
|_google::dense_hash_map::lookup_        |  random|    10000000|    24.4|
|_google::dense_hash_map::erase_         |  random|    10000000|    38.1|
|-                                       |       -|           -|       -|
|_absl::flat_hash_map::insert_           |  random|    10000000|    75.2|
|_absl::flat_hash_map::clear_            |  random|    10000000|     0.7|
|_absl::flat_hash_map::insert_           |  random|    10000000|    71.6|
|_absl::flat_hash_map::lookup_           |  random|    10000000|    33.6|
|_absl::flat_hash_map::erase_            |  random|    10000000|   106.9|

#### 1M update mod n in {1023, 16383, 65535, 1048575} records

Simple benchmark showing the minimum latency for small maps with
constant spread of keys, simulating histogram use-case.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|_std::unordered_map::operator[]_        |    1023|     1000000|    16.1|
|_std::unordered_map::operator[]_        |   16383|     1000000|     7.2|
|_std::unordered_map::operator[]_        |   65535|     1000000|     9.5|
|_std::unordered_map::operator[]_        | 1048575|     1000000|    56.6|
|-                                       |       -|           -|       -|
|_tsl::robin_map::operator[]_            |    1023|     1000000|     6.5|
|_tsl::robin_map::operator[]_            |   16383|     1000000|     1.2|
|_tsl::robin_map::operator[]_            |   65535|     1000000|     1.8|
|_tsl::robin_map::operator[]_            | 1048575|     1000000|    31.8|
|-                                       |       -|           -|       -|
|_zedland::hashmap::operator[]_          |    1023|     1000000|     2.3|
|_zedland::hashmap::operator[]_          |   16383|     1000000|     2.3|
|_zedland::hashmap::operator[]_          |   65535|     1000000|     2.8|
|_zedland::hashmap::operator[]_          | 1048575|     1000000|    18.0|
|-                                       |       -|           -|       -|
|_zedland::linkedhashmap::operator[]_    |    1023|     1000000|     2.2|
|_zedland::linkedhashmap::operator[]_    |   16383|     1000000|     2.5|
|_zedland::linkedhashmap::operator[]_    |   65535|     1000000|     3.1|
|_zedland::linkedhashmap::operator[]_    | 1048575|     1000000|    23.3|
|-                                       |       -|           -|       -|
|_google::dense_hash_map::operator[]_    |    1023|     1000000|     3.4|
|_google::dense_hash_map::operator[]_    |   16383|     1000000|     3.2|
|_google::dense_hash_map::operator[]_    |   65535|     1000000|     3.5|
|_google::dense_hash_map::operator[]_    | 1048575|     1000000|    21.2|
|-                                       |       -|           -|       -|
|_absl::flat_hash_map::operator[]_       |    1023|     1000000|     3.4|
|_absl::flat_hash_map::operator[]_       |   16383|     1000000|     4.9|
|_absl::flat_hash_map::operator[]_       |   65535|     1000000|     9.2|
|_absl::flat_hash_map::operator[]_       | 1048575|     1000000|    62.8|
|-                                       |       -|           -|       -|

#### 1M records - insert, clear, insert, erase, insert, lookup

First insert operation calls resize and rehash, subsequent insert
operations shows insertion speed when table has expanded.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|_std::unordered_map::insert_            |  random|     1000000|   168.6|
|_std::unordered_map::clear_             |  random|     1000000|    36.0|
|_std::unordered_map::insert_            |  random|     1000000|   134.6|
|_std::unordered_map::lookup_            |  random|     1000000|    53.7|
|_std::unordered_map::erase_             |  random|     1000000|   169.2|
|-                                       |       -|           -|       -|
|_tsl::robin_map::insert_                |  random|     1000000|    55.3|
|_tsl::robin_map::clear_                 |  random|     1000000|     7.6|
|_tsl::robin_map::insert_                |  random|     1000000|    20.3|
|_tsl::robin_map::lookup_                |  random|     1000000|    16.2|
|_tsl::robin_map::erase_                 |  random|     1000000|    21.4|
|-                                       |       -|           -|       -|
|_zedland::hashmap::insert_              |  random|     1000000|    42.3|
|_zedland::hashmap::clear_               |  random|     1000000|     1.2|
|_zedland::hashmap::insert_              |  random|     1000000|    17.7|
|_zedland::hashmap::lookup_              |  random|     1000000|    19.0|
|_zedland::hashmap::erase_               |  random|     1000000|    18.0|
|-                                       |       -|           -|       -|
|_zedland::linkedhashmap::insert_        |  random|     1000000|    96.3|
|_zedland::linkedhashmap::clear_         |  random|     1000000|     1.9|
|_zedland::linkedhashmap::insert_        |  random|     1000000|    20.3|
|_zedland::linkedhashmap::lookup_        |  random|     1000000|    21.0|
|_zedland::linkedhashmap::erase_         |  random|     1000000|    69.5|
|-                                       |       -|           -|       -|
|_google::dense_hash_map::insert_        |  random|     1000000|    51.2|
|_google::dense_hash_map::clear_         |  random|     1000000|     0.9|
|_google::dense_hash_map::insert_        |  random|     1000000|    50.9|
|_google::dense_hash_map::lookup_        |  random|     1000000|    18.4|
|_google::dense_hash_map::erase_         |  random|     1000000|    25.1|
|-                                       |       -|           -|       -|
|_absl::flat_hash_map::insert_           |  random|     1000000|    60.8|
|_absl::flat_hash_map::clear_            |  random|     1000000|     1.1|
|_absl::flat_hash_map::insert_           |  random|     1000000|    60.2|
|_absl::flat_hash_map::lookup_           |  random|     1000000|    21.7|
|_absl::flat_hash_map::erase_            |  random|     1000000|    35.5|
