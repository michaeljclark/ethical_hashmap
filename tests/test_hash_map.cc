#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include <map>
#include <random>
#include <chrono>
#include <utility>

#include "hash_map.h"

using namespace std::chrono;

typedef std::pair<uintptr_t,uintptr_t> number_pair_t;

static const number_pair_t numbers[] = {
    { 7, 1 }, { 11, 2 }, { 15, 3 }, { 19, 4 }, { 21, 5 }, { 0, 0 }
};

void test_hash_map_simple()
{
    ethical::hash_map<uintptr_t,uintptr_t> ht;

    for (const number_pair_t *n = numbers; n->first != 0; n++) {
        ht.insert(n->first, n->second);
    }
    for (const number_pair_t *n = numbers; n->first != 0; n++) {
        assert(ht.find(n->first)->second == n->second);
    }
    for (auto ent : ht) {
        for (const number_pair_t *n = numbers; n->first != 0; n++) {
            if (ent.first == n->first) {
                assert(ent.second == n->second);
            }
        }
    }
}

void test_hash_map_delete()
{
    ethical::hash_map<uintptr_t,uintptr_t> ht;
    ht.insert(7, 8);
    assert(ht.find(7)->second == 8);
    ht.erase(7);
    assert(ht.find(7) == ht.end());
}

void test_hash_map_copy()
{
    static const number_pair_t numbers[] = {
        { 666, 4 }, { 777, 1 }, { 888, 2 }, {999, 3}, {0, 0}
    };

    ethical::hash_map<uintptr_t,uintptr_t> hs, ht;
    size_t count;

    ht.insert({666, 4});
    ht.insert({777, 1});
    ht.insert({888, 2});
    ht.insert({999, 3});

    hs = ht;

    count = 0;
    for (auto ent : hs) {
        for (const number_pair_t *n = numbers; n->first != 0; n++) {
            if (ent.first == n->first) {
                assert(ent.second == n->second);
                count++;
            }
        }
    }
    assert(count == 4);

    ethical::hash_map<uintptr_t,uintptr_t> hu(hs);

    count = 0;
    for (auto ent : hu) {
        for (const number_pair_t *n = numbers; n->first != 0; n++) {
            if (ent.first == n->first) {
                assert(ent.second == n->second);
                count++;
            }
        }
    }
    assert(count == 4);
}

void test_hash_map_move()
{
    static const number_pair_t numbers[] = {
        { 666, 4 }, { 777, 1 }, { 888, 2 }, {999, 3}, {0, 0}
    };

    ethical::hash_map<uintptr_t,uintptr_t> ht = std::move(ethical::hash_map<uintptr_t,uintptr_t>());
    size_t count;

    ht.insert({666, 4});
    ht.insert({777, 1});
    ht.insert({888, 2});
    ht.insert({999, 3});

    count = 0;
    for (auto ent : ht) {
        for (const number_pair_t *n = numbers; n->first != 0; n++) {
            if (ent.first == n->first) {
                assert(ent.second == n->second);
                count++;
            }
        }
    }
    assert(count == 4);
}

void test_hash_map_noloop()
{
    ethical::hash_map<int, int> h(4);

    // Fill the table with tombstones.
    for (size_t i = 0; i < 4; i++) {
        h[i];
        h.erase(i);
    }

    // Infinite loop.
    h.find(0);
}

template <typename F>
void insert_random(size_t limit, F fn)
{
    std::random_device random_device;
    std::default_random_engine random_engine;
    std::uniform_int_distribution<uint64_t> random_dist;

    for (size_t i = 0; i < limit; i++) {
        uint64_t key = random_dist(random_engine);
        uint64_t val = random_dist(random_engine);
        fn(key, val);
    }
}

void test_hash_map_random(size_t limit)
{
    ethical::hash_map<uintptr_t,uintptr_t> ht;
    std::map<uintptr_t,uintptr_t> hm;

    insert_random(limit, [&](size_t key, size_t val) {
        ht.insert(key, val);
        hm.insert(hm.end(), std::pair<uintptr_t,uintptr_t>(key, val));
    });

    for (auto &ent : hm) {
        assert(ht.find(ent.first)->second == ent.second);
    }
}

int main(int argc, char **argv)
{
    test_hash_map_simple();
    test_hash_map_delete();
    test_hash_map_noloop();
    test_hash_map_random(1<<16);
    test_hash_map_copy();
    test_hash_map_move();
    return 0;
}