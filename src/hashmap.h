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

#include <utility>
#include <functional>

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

 * This open addressing hashmap uses a 2-bit entry per slot bitmap
 * that eliminates the need for empty and deleted key sentinels.
 * The hashmap has a simple array of key and value pairs and the
 * tombstone bitmap, which are allocated in a single call to malloc.
 */

template <class Key, class Value,
          class _Hash = hash_fnv,
          class _Pred = std::equal_to<Key>>
struct hashmap
{
    /* parameters  */
    static const size_t default_size =    (2<<7);  /* 128 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16); /* 1.0 */

    /* types  */
    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;
    typedef _Pred key_equal;
    typedef _Hash hasher;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    /* functors */
    hasher _hasher;
    key_equal _compare;

    /* storage */
    size_t count;
    size_t limit;
    value_type *data;
    uint64_t *bitmap;

    /*
     * simple iterator
     */
    struct iterator {
        hashmap *h;
        size_t i;

        bool operator==(const iterator &o) const {
            return h == o.h && i == o.i;
        }
        bool operator!=(const iterator &o) const {
            return h != o.h || i != o.i;
        }
        size_t shimmy(size_t i) {
            while (i < h->limit &&
                (bitmap_get(h->bitmap, i) & occupied) != occupied) i++;
            return i;
        }
        iterator& operator++() {
            i = shimmy(i + 1);
            return *this;
        }
        iterator& operator++(int) {
            i = shimmy(i) + 1;
            return *this;
        }
        value_type& operator*() {
            i = shimmy(i);
            return h->data[i];
        }
        value_type* operator->() {
            i = shimmy(i);
            return &h->data[i];
        }
    };
    iterator begin() { return iterator{ this, 0 }; }
    iterator end() { return iterator{ this, limit }; }

    /* utility functions */
    static inline bool is_pow2(intptr_t n) { return  ((n & -n) == n); }

    /*
     * constructors, destructor and simple member functions
     */
    inline hashmap() : hashmap(default_size) {}
    inline hashmap(size_t initial_size) : count(0), limit(initial_size)
    {
        size_t data_size = sizeof(value_type) * initial_size;
        size_t bitmap_size = initial_size >> 2;
        size_t total_size = data_size + bitmap_size;

        assert(is_pow2(initial_size));

        data = (value_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);
        memset(data, 0, total_size);
    }
    inline ~hashmap() { free(data); }

    /*
     * member functions
     */
    inline size_t size() { return count; }
    inline size_t capacity() { return limit; }
    inline size_t load() { return count * load_multiplier / limit; }
    inline size_t index_mask() { return limit - 1; }
    inline size_t hash_index(uint64_t h) { return h & index_mask(); }
    inline size_t key_index(Key key) { return hash_index(_hasher(key)); }

    /*
     * bitmap management
     */
    enum bitmap_state {
        available = 0, occupied = 1, deleted = 2, recycled = 3
    };
    static inline size_t bitmap_idx(size_t i) { return i >> 5; }
    static inline size_t bitmap_shift(size_t i) { return ((i << 1) & 63); }
    static inline bitmap_state bitmap_get(uint64_t *bitmap, size_t i)
    {
        return (bitmap_state)((bitmap[bitmap_idx(i)] >> bitmap_shift(i)) & 3);
    }
    static inline void bitmap_set(uint64_t *bitmap, size_t i, uint64_t value)
    {
        bitmap[bitmap_idx(i)] |= (value << bitmap_shift(i));
    }
    static inline void bitmap_clear(uint64_t *bitmap, size_t i, uint64_t value)
    {
        bitmap[bitmap_idx(i)] &= ~(value << bitmap_shift(i));
    }

    /*
     * resize to expand the hashtable storage
     */
    void resize_internal(value_type *old_data, uint64_t *old_bitmap,
                         size_t old_size, size_t new_size)
    {
        size_t data_size = sizeof(value_type) * new_size;
        size_t bitmap_size = new_size >> 2;
        size_t total_size = data_size + bitmap_size;

        assert(is_pow2(new_size));

        data = (value_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);
        limit = new_size;
        memset(data, 0, total_size);

        size_t i = 0;
        for (value_type *v = old_data; v != old_data + old_size; v++, i++) {
            if ((bitmap_get(old_bitmap, i) & occupied) == occupied) {
                insert(v->first, v->second);
            }
        }

        free(old_data);
    }

    /**
     * clear the table
     */
    void clear()
    {
        size_t data_size = sizeof(value_type) * limit;
        size_t bitmap_size = limit >> 2;
        size_t total_size = data_size + bitmap_size;
        memset(data, 0, total_size);
        count = 0;
    }

    /**
     * insert element
     *
     * @param key to insert
     * @param val to insert
     */
    iterator insert(iterator i, const value_type& value)
    {
        return insert(value);
    }

    /**
     * insert element
     *
     * @param key to insert
     * @param val to insert
     */
    iterator insert(Key key, Value val)
    {
        return insert(value_type(key, val));
    }

    /**
     * insert element
     *
     * @param value_type to insert
     */
    iterator insert(const value_type& value)
    {
        size_t i = key_index(value.first);
        for (;;) {
            if ((bitmap_get(bitmap, i) & occupied) != occupied) {
                bitmap_set(bitmap, i, occupied);
                data[i] = value;
                count++;
                if (load() > load_factor) {
                    resize_internal(data, bitmap, limit, limit << 1);
                    return find(value.first);
                } else {
                    return iterator{this, i};
                }
            } else if (_compare(data[i].first, value.first)) {
                data[i].second = value.second;
                return iterator{this, i};
            }
            i = (i + 1) & index_mask();
        }
    }

    /**
     * access element
     *
     * @param Key to find
     * @returns iterator to the element or end if not found
     */
    Value& operator[](const Key &key)
    {
        size_t i = key_index(key);
        for (;;) {
            if ((bitmap_get(bitmap, i) & occupied) != occupied) {
                bitmap_set(bitmap, i, occupied);
                data[i].first = key;
                count++;
                if (load() > load_factor) {
                    resize_internal(data, bitmap, limit, limit << 1);
                    i = key_index(key);
                }
                return data[i].second;
            } else if (_compare(data[i].first, key)) {
                return data[i].second;
            }
            i = (i + 1) & index_mask();
        }
    }

    /**
     * find iterator to element
     *
     * @param Key to find
     * @returns iterator to the element or to end()
     */
    iterator find(const Key &key)
    {
        size_t i = key_index(key);
        for (;;) {
            bitmap_state t = bitmap_get(bitmap, i);
                 if (t == available)       /* notfound */ break;
            else if (t == deleted);        /* skip */
            else if (_compare(data[i].first, key)) return iterator{this, i};
            i = (i + 1) & index_mask();
        }
        return end();
    }

    /**
     * erase element
     *
     * @param Key to erase
     */
    void erase(Key key)
    {
        size_t i = key_index(key);
        for (;;) {
            bitmap_state t = bitmap_get(bitmap, i);
                 if (t == available)       /* notfound */ break;
            else if (t == deleted);        /* skip */
            else if (_compare(data[i].first, key)) {
                bitmap_set(bitmap, i, deleted);
                data[i].second = Value(0);
                bitmap_clear(bitmap, i, occupied);
                count--;
                return;
            }
            i = (i + 1) & index_mask();
        }
    }
};
