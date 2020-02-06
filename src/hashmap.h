/*
 * hashmap - fast open addressing hash table with FNV hash function
 *
 * Copyright (c) 2020 Michael Clark <michaeljclark@mac.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <cassert>

#include <tuple>
#include <utility>

/*
 * FNV-1a hash algorithm
 *
 * The 64-bit word hash uses a rotate of the word so that
 * entropy in the key is permuted through all bit positions.
 */

struct hash_fnv
{
    static const uint64_t fnv_base = 0xcbf29ce484222325;
    static const uint64_t fnv_prime = 0x100000001b3;

    /* FNV-1a hash function */
    uint64_t operator()(const char *s) const
    {
        uint64_t h = fnv_base;
        while (*s) {
            h = h ^ *s++;
            h = h * fnv_prime;
        }
        return h;
    }

    static inline uint64_t ror(uint64_t n, size_t s) {
        return ((n >> s) | (n << (64-s)));
    }

    /* FNV-1amc hash function */
    uint64_t operator()(uint64_t r) const
    {
        uint64_t h = fnv_base;
        for (size_t i = 0; i < 64; i += 8) {
            /* xor rotated 64-bit word (8x8 permute) */
            h = h ^ ror(r, i);
            h = h * fnv_prime;
        }
        return h;
    }
};


/*
 * Identity hash function
 *
 * The low complexity identity hash function works very well with
 * open-addressing hash tables, effectively making the table operate
 * like an array when keys are less than the table size.
 */

struct hash_ident
{
    uint64_t operator()(uint64_t r) const { return r; }
};


/*
 * hashmap - Fast open addressing hash map with tombstone bit map.

 * This open addressing hashmap uses a 2-bit per slot tombstone map
 * eliminating any requirement for empty and deleted key sentinels.
 * The hashmap has a simple array of key and value pairs and the
 * tombstone bitmap, which are allocated in a single call to malloc.
 */

template <typename Key, typename Value, typename _Hash = hash_fnv>
struct hashmap
{
    static const _Hash hasher;

    static const size_t default_size =    (2<<10); /* 1024 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16);

    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;

    size_t count;
    size_t limit;
    value_type *data;
    uint64_t *tombs;

    enum tomb_state {
        available = 0,
        occupied = 1,
        deleted = 2,
        recycled = 3
    };

    static inline size_t tomb_idx(size_t idx) { return idx >> 5; }
    static inline size_t tomb_shift(size_t idx) { return ((idx << 1) & 63); }

    static inline tomb_state tomb_get(uint64_t *tombs, size_t idx) {
        return (tomb_state)((tombs[tomb_idx(idx)] >> tomb_shift(idx)) & 3);
    }
    static inline void tomb_set(uint64_t *tombs, size_t idx, uint64_t value) {
        tombs[tomb_idx(idx)] |= (value << tomb_shift(idx));
    }
    static inline void tomb_clear(uint64_t *tombs, size_t idx, uint64_t value) {
        tombs[tomb_idx(idx)] &= ~(value << tomb_shift(idx));
    }

    struct iterator {
        hashmap *h;
        size_t idx;

        bool operator==(const iterator &o) const {
            return std::tie(h, idx) == std::tie(o.h, o.idx);
        }
        bool operator!=(const iterator &o) const {
            return std::tie(h, idx) != std::tie(o.h, o.idx);
        }
        size_t shimmy(size_t idx) {
            while (idx < h->limit &&
                (tomb_get(h->tombs, idx) & occupied) != occupied) idx++;
            return idx;
        }
        iterator& operator++() {
            idx = shimmy(idx + 1);
            return *this;
        }
        value_type* operator*() {
            idx = shimmy(idx);
            return &h->data[idx];
        }
    };

    iterator begin() { return iterator{ this, 0 }; }
    iterator end() { return iterator{ this, limit }; }

    inline hashmap() : hashmap(default_size) {}

    inline hashmap(size_t initial_size) : count(0), limit(initial_size)
    {
        size_t data_size = sizeof(value_type) * initial_size;
        size_t tomb_size = initial_size >> 2;
        size_t total_size = data_size + tomb_size;
        data = (value_type*)malloc(total_size);
        tombs = (uint64_t*)((char*)data + data_size);
        memset(data, 0, total_size);
    }

    inline ~hashmap() { free(data); }

    inline size_t size() { return count; }
    inline size_t capacity() { return limit; }
    inline size_t load() { return count * load_multiplier / limit; }
    inline size_t index_mask() { return limit - 1; }
    inline size_t hash_index(uint64_t h) { return h & index_mask(); }
    inline size_t key_index(Key key) { return hash_index(hasher(key)); }

    static inline bool is_pow2(intptr_t n) { return  ((n & -n) == n); }

    void resize(value_type *old_data, uint64_t *old_tombs,
                size_t old_size, size_t new_size)
    {
        size_t data_size = sizeof(value_type) * new_size;
        size_t tomb_size = new_size >> 2;
        size_t total_size = data_size + tomb_size;

        assert(is_pow2(new_size));

        data = (value_type*)malloc(total_size);
        tombs = (uint64_t*)((char*)data + data_size);
        limit = new_size;
        memset(data, 0, total_size);

        size_t idx = 0;
        for (value_type *ent = old_data; ent != old_data + old_size; ent++, idx++) {
            if ((tomb_get(old_tombs, idx) & occupied) == occupied) {
                insert(ent->first, ent->second);
            }
        }

        free(old_data);
    }

    void insert(Key key, Value val)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
            if (t == available || t == deleted) {
                /* available */
                tomb_set(tombs, idx, occupied);
                data[idx].first = key;
                data[idx].second = val;
                count++;
                if (load() > load_factor) {
                    resize(data, tombs, limit, limit << 1);
                }
                return;
            } else if (data[idx].first == key) {  /* found */
                data[idx].second = val;
                return;
            }
            idx = (idx + 1) & index_mask();
        }
    }

    Value& operator[](const Key &key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
            if (t == available || t == deleted) {
                /* available */
                tomb_set(tombs, idx, occupied);
                data[idx].first = key;
                count++;
                if (load() > load_factor) {
                    resize(data, tombs, limit, limit << 1);
                    idx = key_index(key);
                }
                return data[idx].second;
            } else if (data[idx].first == key) {  /* found */
                return data[idx].second;
            }
            idx = (idx + 1) & index_mask();
        }
    }

    Value lookup(Key key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
                 if (t == available)           /* notfound */ break;
            else if (t == deleted);            /* skip */
            else if (data[idx].first == key)     /* found */ return data[idx].second;
            idx = (idx + 1) & index_mask();
        }
        return Value(0);
    }

    void erase(Key key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
                 if (t == available)           /* notfound */ break;
            else if (t == deleted);            /* skip */
            else if (data[idx].first == key) {   /* found */
                /* { recycled, occupied } -> deleted */
                tomb_set(tombs, idx, deleted);
                tomb_clear(tombs, idx, occupied);
                data[idx].second = Value(0);
                count--;
                return;
            }
            idx = (idx + 1) & index_mask();
        }
    }
};
