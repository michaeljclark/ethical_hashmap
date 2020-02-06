#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include <map>
#include <random>
#include <chrono>
#include <utility>

#include "hashmap.h"

using namespace std::chrono;

typedef std::pair<uintptr_t,uintptr_t> number_pair_t;

static const number_pair_t numbers[] = {
    { 7, 1 }, { 11, 2 }, { 15, 3 }, { 19, 4 }, { 21, 5 }, { 0, 0}
};

void test_hashmap_simple()
{
    zhashmap<uintptr_t,uintptr_t> ht;
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

void test_hashmap_random(size_t limit)
{
    zhashmap<uintptr_t,uintptr_t> ht;
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
    test_hashmap_simple();
    test_hashmap_random(1<<16);
    return 0;
}