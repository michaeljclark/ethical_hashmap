/*
 * hashmap - fast map using FNV hash function
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

struct hash_ident
{
    uint64_t operator()(uint64_t r) const { return r; }
};

template <typename K, typename V, typename _Hash = hash_fnv>
struct hashmap
{
    static const _Hash hasher;

    static const size_t default_size =    (2<<10); /* 1024 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16);

    struct entry {
        K key;
        V val;
    };

    size_t count;
    size_t limit;
    entry *data;
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

    inline hashmap() : hashmap(default_size) {}

    inline hashmap(size_t initial_size) : count(0), limit(initial_size)
    {
        size_t data_size = sizeof(entry) * initial_size;
        size_t tomb_size = initial_size >> 2;
        size_t total_size = data_size + tomb_size;
        data = (entry*)malloc(total_size);
        tombs = (uint64_t*)((char*)data + data_size);
        memset(data, 0, total_size);
    }

    inline ~hashmap() { free(data); }

    inline size_t size() { return count; }
    inline size_t capacity() { return limit; }
    inline size_t load() { return count * load_multiplier / limit; }
    inline size_t index_mask() { return limit - 1; }
    inline size_t hash_index(uint64_t h) { return h & index_mask(); }
    inline size_t key_index(K key) { return hash_index(hasher(key)); }

    static inline bool is_pow2(intptr_t n) { return  ((n & -n) == n); }

    void resize(entry *old_data, uint64_t *old_tombs,
                size_t old_size, size_t new_size)
    {
        size_t data_size = sizeof(entry) * new_size;
        size_t tomb_size = new_size >> 2;
        size_t total_size = data_size + tomb_size;

        assert(is_pow2(new_size));

        data = (entry*)malloc(total_size);
        tombs = (uint64_t*)((char*)data + data_size);
        limit = new_size;
        memset(data, 0, total_size);

        size_t idx = 0;
        for (entry *ent = old_data; ent != old_data + old_size; ent++, idx++) {
            if ((tomb_get(old_tombs, idx) & occupied) == occupied) {
                insert(ent->key, ent->val);
            }
        }

        free(old_data);
    }

    void insert(K key, V val)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
            if (t == available || t == deleted) {
                /* available */
                tomb_set(tombs, idx, occupied);
                data[idx].key = key;
                data[idx].val = val;
                count++;
                if (load() > load_factor) {
                    resize(data, tombs, limit, limit << 1);
                }
                return;
            } else if (data[idx].key == key) {  /* found */
                data[idx].val = val;
                return;
            }
            idx = (idx + 1) & index_mask();
        }
    }

    V& operator[](const K &key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
            if (t == available || t == deleted) {
                /* available */
                tomb_set(tombs, idx, occupied);
                data[idx].key = key;
                count++;
                if (load() > load_factor) {
                    resize(data, tombs, limit, limit << 1);
                    idx = key_index(key);
                }
                return data[idx].val;
            } else if (data[idx].key == key) {  /* found */
                return data[idx].val;
            }
            idx = (idx + 1) & index_mask();
        }
    }

    V lookup(K key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
                 if (t == available)           /* notfound */ break;
            else if (t == deleted);            /* skip */
            else if (data[idx].key == key)     /* found */ return data[idx].val;
            idx = (idx + 1) & index_mask();
        }
        return V(0);
    }

    void erase(K key)
    {
        size_t idx = key_index(key);
        for (;;) {
            tomb_state t = tomb_get(tombs, idx);
                 if (t == available)           /* notfound */ break;
            else if (t == deleted);            /* skip */
            else if (data[idx].key == key) {   /* found */
                /* { recycled, occupied } -> deleted */
                tomb_set(tombs, idx, deleted);
                tomb_clear(tombs, idx, occupied);
                data[idx].val = V(0);
                count--;
                return;
            }
            idx = (idx + 1) & index_mask();
        }
    }
};
