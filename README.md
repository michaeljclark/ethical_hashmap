# ethical hash map

High performance, compact and ethical C++ hash sets and maps:

> ### _hash_set_
> - Fast open addressing hash set tuned for small sets.
>
> ### _hash_map_
> - Fast open addressing hash map tuned for small maps.
>
> ### _linked_hash_set_
> - Fast open addressing hash set with link list tuned
>   for small maps with predictable iteration order.
>
> ### _linked_hash_map_
> - Fast open addressing hash map with link list tuned
>   for small maps with predictable iteration order.

These hash sets and maps are open-addressing hashtables similar
to `google/dense_hash_map`, but they use tombstone bitmaps to
eliminate the necessity for empty or deleted key sentinels.

These containers implement most of the C++ unordered associative
container requrements thus can be substituted for `unordered_map`
and typical use cases including C++11 _for-loop_.

There is currently no equivalent to `linked_hash_map` in the STL so it
is perhaps somewhat like an `ordered_map` that uses the _pos_ iterator
hint in the first argument of _insert_ to set the position of a new
value inserted into the linked list, while _find_ uses the hashed key
index to locate _key value pairs_ in the hash table.

## Design 

These maps are designed for a small memory footprint. There is one
structure and one heap allocation which is divided between an array
of _tuples_ composed of _(Key, Value)_ pairs and a tombstone bitmap.
_Key_ is hashed to index into the open addressing array, and
collisions are handled by skipping to the next available slot.

Deletions requires the use of tombstones. Sentinel key values occupy
key-space, and make containers incompatible with associative container
requirements, thus a 2-bit tombstone array is used. 2-bits are used
to distinguish _available_, _occupied_, and _deleted_ states.

```
    enum bitmap_state {
        available = 0, occupied = 1, deleted = 2, recycled = 3
    };
```

_linked_hash_map_ adds _(next, prev)_ indices to the array _tuple_,
_(head, tail)_ indices to the structure.

### Memory usage

The follow table shows memory usage for the default 16 slot map
_(table units are in bytes)_:

| map                           |     struct | malloc |
|:----------------------------- | ----------:| ------:|
|_hash_set<u32>_                |         40 |     68 |
|_hash_set<u64>_                |         40 |    132 |
|_hash_map<u32,u32>_            |         40 |    132 |
|_hash_map<u64,u64>_            |         40 |    260 |
|_linked_hash_set<u32,i32>_     |         48 |    198 |
|_linked_hash_set<u64,i64>_     |         56 |    388 |
|_linked_hash_map<u32,u32,i32>_ |         48 |    260 |
|_linked_hash_map<u64,u64,i64>_ |         56 |    516 |

The following table shows the structure size in bytes on _x86_64_:

| map                               | size |
|:----------------------------------| ----:|
|_sizeof(hash_map)_                 |   40 |
|_sizeof(linked_hash_map)_          |   48 |
|_sizeof(absl::flat_hash_map)_      |   48 |
|_sizeof(std::unordered_map)_       |   56 |
|_sizeof(tsl::robin_map)_           |   80 |
|_sizeof(google::dense_hash_map)_   |   88 |

_linked_hash_map_ by default uses 32-bit integers for indices,
limiting it to 2^31 entries, however, it can be instantiated
with alternative integer types for indices.

## Build Instructions

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```
