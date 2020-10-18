# zhashmap

High performance compact C++ hashmaps:

- `hashmap`       - _open addressing hash table._
- `linkedhashmap` - _open addressing hash table and
                     combined bidirectional link-list._

## Overview

These hashmaps are open-addressing hashtables similar to
`google/dense_hash_map`, but they use tombstone bitmaps to
eliminate necessity for empty or deleted key sentinels.

- _hashmap_
  - Fast open addressing hash table tuned for small maps such as
    for histograms and other performance critical maps.
- _linkedhashmap_
  - Fast open addressing hash table with bidirectional link list
    tuned for small maps that need predictable iteration order as
    well as high performance.

These containers implement most of the C++ unordered associative
container requrements thus can be substituted for `unordered_map`
and typical use cases including C++11 _for-loop_.

There is currently no equivalent to `linkedhashmap` in the STL so it
is perhaps somewhat of an `ordered_map` implementation.


## Design 

These maps are designed for small memory foorprint. There is one
structure and one heap allocation which is divided between an array
of _tuples_ composed of _(Key, Value)_ and a tombstone bitmap. _Key_
is hashed to index into the open addressing array, and collisions are
handled by skipping to the next available slot.

Deletions requires the use of tombstones. Sentinel key values occupy
key-space, and make containers incompatible with associative container
requirements, thus a 2-bit tombstone array is used. 2-bits are used
to distinguish _available_, _occupied_, and _deleted_ states.

``
    enum bitmap_state {
        available = 0, occupied = 1, deleted = 2, recycled = 3
    };
``

_linkedhashmap_ adds _(next, prev)_ indices to the array _tuple_,
_(head, tail)_ indices to the structure.

The source code has almost zero comments but is written to be concise
and easy to understand.

### Memory usage

The follow table shows memory usage for the default 16 slot map
_(table units are in bytes)_:

| map            |     struct | (data+bmap) | malloc |
|:-------------- | ----------:| -----------:| ------:|
|`hashmap`       |         32 |     (256+4) |    260 |
|`linkedhashmap` |         40 |     (384+4) |    388 |

### Data structure

These extracts from the source show the array data structure.

#### _hashmap_ parameters and table structure

```
template <class Key, class Value,
          class _Hash = std::hash<Key>,
          class _Pred = std::equal_to<Key>>
struct hashmap
{
    static const size_t default_size =    (2<<3);  /* 16 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16); /* 1.0 */
    ...
    struct data_type {
        Key first;
        Value second;
    };	
}
```

#### _linkedhashmap_ parameters and table structure

```
template <class Key, class Value,
          class _Hash = std::hash<Key>,
          class _Pred = std::equal_to<Key>,
          class _Offset = int32_t>
struct linkedhashmap
{
    static const size_t default_size =    (2<<3);  /* 16 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16); /* 1.0 */
    ...
    struct data_type {
        Key first;
        Value second;
        _Offset prev;
        _Offset next;
    };
}
```

## Benchmarks

The following benchmarks were performed on an _Intel Core i9-7980XE_
with GCC 9.2.

These benchmarks are not exhaustive. It can be seen that the maps have
different performance characteristics depending on use. Benchmarks show
the implementation is competitive with `tsl::robin_map`, probably the
fastest C++ hashtable in existence, and consistently faster than
`google::dense_hash_map` and `absl::flat_hash_map`.

There are many cases where `zedland::hashmap` is the fastest map.

#### update 10M mod 16383 records

Simple benchmark intended to show the access latency for small maps
with relatively constant set of keys, such as use for histograms.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`tsl::robin_map::operator[]`            |   16383|    10000000|     0.9|
|`zedland::hashmap::operator[]`          |   16383|    10000000|     1.4|
|`zedland::linkedhashmap::operator[]`    |   16383|    10000000|     2.0|
|`google::dense_hash_map::operator[]`    |   16383|    10000000|     2.1|
|`absl::flat_hash_map::operator[]`       |   16383|    10000000|     4.0|
|`std::unordered_map::operator[]`        |   16383|    10000000|     6.3|

#### 10M records - insert, clear, insert, erase, insert, lookup

The first insert operation includes resize and rehash. The subsequent
insert operations shows insertion speed when the table has expanded.

|container                               |  spread|       count| time_ns|
|:-------------------------------------- |  -----:|       ----:| ------:|
|`std::unordered_map::insert`            |  random|    10000000|   380.8|
|`std::unordered_map::clear`             |  random|    10000000|    68.3|
|`std::unordered_map::insert`            |  random|    10000000|   223.5|
|`std::unordered_map::erase`             |  random|    10000000|   263.8|
|`std::unordered_map::insert`            |  random|    10000000|   223.9|
|`std::unordered_map::lookup`            |  random|    10000000|    72.9|
|-                                       |       -|           -|       -|
|`tsl::robin_map::insert`                |  random|    10000000|    87.1|
|`tsl::robin_map::clear`                 |  random|    10000000|     9.8|
|`tsl::robin_map::insert`                |  random|    10000000|    27.5|
|`tsl::robin_map::erase`                 |  random|    10000000|    27.8|
|`tsl::robin_map::insert`                |  random|    10000000|    28.5|
|`tsl::robin_map::lookup`                |  random|    10000000|    19.8|
|-                                       |       -|           -|       -|
|`zedland::hashmap::insert`              |  random|    10000000|    67.3|
|`zedland::hashmap::clear`               |  random|    10000000|     2.2|
|`zedland::hashmap::insert`              |  random|    10000000|    23.1|
|`zedland::hashmap::erase`               |  random|    10000000|    29.7|
|`zedland::hashmap::insert`              |  random|    10000000|    23.2|
|`zedland::hashmap::lookup`              |  random|    10000000|    27.4|
|-                                       |       -|           -|       -|
|`zedland::linkedhashmap::insert`        |  random|    10000000|   270.2|
|`zedland::linkedhashmap::clear`         |  random|    10000000|     3.4|
|`zedland::linkedhashmap::insert`        |  random|    10000000|    26.4|
|`zedland::linkedhashmap::erase`         |  random|    10000000|   113.4|
|`zedland::linkedhashmap::insert`        |  random|    10000000|    26.9|
|`zedland::linkedhashmap::lookup`        |  random|    10000000|    26.2|
|-                                       |       -|           -|       -|
|`google::dense_hash_map::insert`        |  random|    10000000|    83.0|
|`google::dense_hash_map::clear`         |  random|    10000000|     1.0|
|`google::dense_hash_map::insert`        |  random|    10000000|    83.0|
|`google::dense_hash_map::erase`         |  random|    10000000|    36.9|
|`google::dense_hash_map::insert`        |  random|    10000000|    93.2|
|`google::dense_hash_map::lookup`        |  random|    10000000|    24.6|
|-                                       |       -|           -|       -|
|`absl::flat_hash_map::insert`           |  random|    10000000|    70.6|
|`absl::flat_hash_map::clear`            |  random|    10000000|     0.7|
|`absl::flat_hash_map::insert`           |  random|    10000000|    72.2|
|`absl::flat_hash_map::erase`            |  random|    10000000|   106.7|
|`absl::flat_hash_map::insert`           |  random|    10000000|    55.6|
|`absl::flat_hash_map::lookup`           |  random|    10000000|    33.9|

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
