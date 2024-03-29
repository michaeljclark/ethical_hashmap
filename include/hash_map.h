/*
 * Fast open addressing hash table with tombstone bit map.
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

namespace ethical {

/*
 * This open addressing hash_map uses a 2-bit entry per slot bitmap
 * that eliminates the need for empty and deleted key sentinels.
 * The hash_map has a simple array of key and value pairs and the
 * tombstone bitmap, which are allocated in a single call to malloc.
 */

template <class Key, class Value,
          class Hash = std::hash<Key>,
          class Pred = std::equal_to<Key>>
struct hash_map
{
    static const size_t default_size =    (2<<3);  /* 16 */
    static const size_t load_factor =     (2<<15); /* 0.5 */
    static const size_t load_multiplier = (2<<16); /* 1.0 */

    static inline Hash _hasher;
    static inline Pred _compare;

    struct data_type {
        Key first;
        Value second;
    };

    typedef Key key_type;
    typedef Value mapped_type;
    typedef std::pair<Key, Value> value_type;
    typedef Hash hasher;
    typedef Pred key_equal;
    typedef data_type& reference;
    typedef const data_type& const_reference;

    size_t used;
    size_t tombs;
    size_t limit;
    data_type *data;
    uint64_t *bitmap;

    /*
     * scanning iterator
     */

    struct iterator
    {
        hash_map *h;
        size_t i;

        size_t step(size_t i) {
            while (i < h->limit &&
                   (bitmap_get(h->bitmap, i) & occupied) != occupied) i++;
            return i;
        }
        iterator& operator++() { i = step(i+1); return *this; }
        iterator operator++(int) { iterator r = *this; ++(*this); return r; }
        data_type& operator*() { i = step(i); return h->data[i]; }
        data_type* operator->() { i = step(i); return &h->data[i]; }
        bool operator==(const iterator &o) const { return h == o.h && i == o.i; }
        bool operator!=(const iterator &o) const { return h != o.h || i != o.i; }
    };

    /*
     * constructors and destructor
     */

    inline hash_map() : hash_map(default_size) {}
    inline hash_map(size_t initial_size) :
        used(0), tombs(0), limit(initial_size)
    {
        size_t data_size = sizeof(data_type) * limit;
        size_t bitmap_size = bitmap_capacity(limit);
        size_t total_size = data_size + bitmap_size;

        assert(is_pow2(limit));

        data = (data_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);
        memset(bitmap, 0, bitmap_size);
    }

    inline ~hash_map()
    {
        if (data) {
            for (size_t i = 0; i < limit; i++) {
                if ((bitmap_get(bitmap, i) & occupied) == occupied) {
                    data[i].~data_type();
                }
            }
            free(data);
        }
    }

    /*
     * copy constructor and assignment operator
     */

    inline hash_map(const hash_map &o) :
        used(o.used), tombs(o.tombs), limit(o.limit)
    {
        size_t data_size = sizeof(data_type) * limit;
        size_t bitmap_size = bitmap_capacity(limit);
        size_t total_size = data_size + bitmap_size;

        data = (data_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);

        memcpy(bitmap, o.bitmap, bitmap_size);
        for (size_t i = 0; i < limit; i++) {
            if ((bitmap_get(bitmap, i) & occupied) == occupied) {
                data[i] = /* copy */ o.data[i];
            }
        }
    }

    inline hash_map(hash_map &&o) :
        used(o.used), tombs(o.tombs), limit(o.limit),
        data(o.data), bitmap(o.bitmap)
    {
        o.data = nullptr;
        o.bitmap = nullptr;
    }

    inline hash_map& operator=(const hash_map &o)
    {
        free(data);

        used = o.used;
        tombs = o.tombs;
        limit = o.limit;

        size_t data_size = sizeof(data_type) * limit;
        size_t bitmap_size = bitmap_capacity(limit);
        size_t total_size = data_size + bitmap_size;

        data = (data_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);

        memcpy(bitmap, o.bitmap, bitmap_size);
        for (size_t i = 0; i < limit; i++) {
            if ((bitmap_get(bitmap, i) & occupied) == occupied) {
                data[i] = /* copy */ o.data[i];
            }
        }

        return *this;
    }

    inline hash_map& operator=(hash_map &&o)
    {
        data = o.data;
        bitmap = o.bitmap;
        used = o.used;
        tombs = o.tombs;
        limit = o.limit;

        o.data = nullptr;
        o.bitmap = nullptr;

        return *this;
    }

    /*
     * member functions
     */

    inline size_t size() { return used; }
    inline size_t capacity() { return limit; }
    inline size_t load() { return (used + tombs) * load_multiplier / limit; }
    inline size_t index_mask() { return limit - 1; }
    inline size_t hash_index(uint64_t h) { return h & index_mask(); }
    inline size_t key_index(Key key) { return hash_index(_hasher(key)); }
    inline hasher hash_function() const { return _hasher; }
    inline iterator begin() { return iterator{ this, 0 }; }
    inline iterator end() { return iterator{ this, limit }; }

    /*
     * bit manipulation helpers
     */

    enum bitmap_state {
        available = 0, occupied = 1, deleted = 2, recycled = 3
    };
    static inline size_t bitmap_capacity(size_t limit)
    {
        return (((limit + 3) >> 2) + 7) & ~7;
    }
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
    static inline bool is_pow2(intptr_t n) { return  ((n & -n) == n); }

    /**
     * the implementation
     */

    void resize_internal(data_type *old_data, uint64_t *old_bitmap,
                         size_t old_limit, size_t new_limit)
    {
        size_t data_size = sizeof(data_type) * new_limit;
        size_t bitmap_size = bitmap_capacity(new_limit);
        size_t total_size = data_size + bitmap_size;

        assert(is_pow2(new_limit));

        data = (data_type*)malloc(total_size);
        bitmap = (uint64_t*)((char*)data + data_size);
        limit = new_limit;
        memset(bitmap, 0, bitmap_size);

        size_t i = 0;
        for (data_type *v = old_data; v != old_data + old_limit; v++, i++) {
            if ((bitmap_get(old_bitmap, i) & occupied) != occupied) continue;
            for (size_t j = key_index(v->first); ; j = (j+1) & index_mask()) {
                if ((bitmap_get(bitmap, j) & occupied) != occupied) {
                    bitmap_set(bitmap, j, occupied);
                    new (&data[j]) data_type();
                    data[j] = /* copy */ *v;
                    break;
                }
            }
        }

        tombs = 0;
        for (size_t i = 0; i < old_limit; i++) {
            if ((bitmap_get(old_bitmap, i) & occupied) == occupied) {
                old_data[i].~data_type();
            }
        }
        free(old_data);
    }

    void clear()
    {
        for (size_t i = 0; i < limit; i++) {
            if ((bitmap_get(bitmap, i) & occupied) == occupied) {
                data[i].~data_type();
            }
        }
        size_t bitmap_size = bitmap_capacity(limit);
        memset(bitmap, 0, bitmap_size);
        used = tombs = 0;
    }

    iterator insert(iterator i, const value_type& val) { return insert(val); }
    iterator insert(Key key, Value val) { return insert(value_type(key, val)); }

    iterator insert(const value_type& v)
    {
        for (size_t i = key_index(v.first); ; i = (i+1) & index_mask()) {
            bitmap_state state = bitmap_get(bitmap, i);
            if ((state & recycled) == available) {
                bitmap_set(bitmap, i, occupied);
                new (&data[i]) data_type();
                data[i] = /* copy */ data_type{v.first, v.second};
                used++;
                if ((state & deleted) == deleted) tombs--;
                if (load() > load_factor) {
                    resize_internal(data, bitmap, limit, limit << 1);
                    for (i = key_index(v.first); ; i = (i+1) & index_mask()) {
                        bitmap_state state = bitmap_get(bitmap, i);
                             if (state == available) abort();
                        else if (state == deleted); /* skip */
                        else if (_compare(data[i].first, v.first)) {
                            return iterator{this, i};
                        }
                    }
                } else {
                    return iterator{this, i};
                }
            } else if (_compare(data[i].first, v.first)) {
                data[i].second = /* copy */ v.second;
                return iterator{this, i};
            }
        }
    }

    Value& operator[](const Key &key)
    {
        for (size_t i = key_index(key); ; i = (i+1) & index_mask()) {
            bitmap_state state = bitmap_get(bitmap, i);
            if ((state & recycled) == available) {
                bitmap_set(bitmap, i, occupied);
                new (&data[i]) data_type();
                data[i].first = /* copy */ key;
                used++;
                if ((state & deleted) == deleted) tombs--;
                if (load() > load_factor) {
                    resize_internal(data, bitmap, limit, limit << 1);
                    for (i = key_index(key);; i = (i+1) & index_mask()) {
                        bitmap_state state = bitmap_get(bitmap, i);
                             if (state == available) abort();
                        else if (state == deleted); /* skip */
                        else if (_compare(data[i].first, key)) {
                            return data[i].second;
                        }
                    }
                }
                return data[i].second;
            } else if (_compare(data[i].first, key)) {
                return data[i].second;
            }
        }
    }

    iterator find(const Key &key)
    {
        for (size_t i = key_index(key); ; i = (i+1) & index_mask()) {
            bitmap_state state = bitmap_get(bitmap, i);
                 if (state == available)           /* notfound */ break;
            else if (state == deleted);            /* skip */
            else if (_compare(data[i].first, key)) return iterator{this, i};
        }
        return end();
    }

    void erase(Key key)
    {
        for (size_t i = key_index(key); ; i = (i+1) & index_mask()) {
            bitmap_state state = bitmap_get(bitmap, i);
                 if (state == available)           /* notfound */ break;
            else if (state == deleted);            /* skip */
            else if (_compare(data[i].first, key)) {
                bitmap_set(bitmap, i, deleted);
                data[i].~data_type();
                bitmap_clear(bitmap, i, occupied);
                used--;
                tombs++;
                return;
            }
        }
    }

    bool operator==(const hash_map &o) const
    {
        for (auto i : const_cast<hash_map&>(*this)) {
            auto j = const_cast<hash_map&>(o).find(i.first);
            if (j == const_cast<hash_map&>(o).end()) return false;
            if (i.second != j->second) return false;
        }
        for (auto i : const_cast<hash_map&>(o)) {
            auto j = const_cast<hash_map&>(*this).find(i.first);
            if (j == const_cast<hash_map&>(*this).end()) return false;
            if (i.second != j->second) return false;
        }
        return true;
    }

    bool operator!=(const hash_map &o) const { return !(*this == o); }
};

};