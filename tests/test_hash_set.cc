#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include <array>
#include <utility>
#include <initializer_list>

#include "sha256.h"
#include "hash_map.h"
#include "hash_set.h"

struct hash_hash_map
{
    typedef zedland::hash_map<int,int> hash_map_t;

    size_t operator()(hash_map_t &m)
    {
        uint8_t b[32];
        sha256_ctx ctx;
        sha256_init(&ctx);
        for (auto &ent : m) {
            sha256_update(&ctx, (const void*)&ent.first, sizeof(ent.first));
            sha256_update(&ctx, (const void*)&ent.second, sizeof(ent.second));
        }
        sha256_final(&ctx, b);
        return *(size_t*)b;
    }
};

void test_hash_set_simple()
{
    static const uintptr_t numbers[] = { 8, 9, 6, 7, 4, 5, 2, 3, 0 };

    zedland::hash_set<uintptr_t> ht;

    for (const uintptr_t *n = numbers; *n != 0; n++) {
        ht.insert(*n);
    }
    for (const uintptr_t *n = numbers; *n != 0; n++) {
        assert(ht.find(*n)->first == *n);
    }
    const uintptr_t *n = numbers;
    for (uintptr_t n : numbers) {
        if (n > 0) assert(ht.find(n) != ht.end());
    }
}

zedland::hash_map<int,int>
make_map(std::initializer_list<std::pair<int,int>> l)
{
    zedland::hash_map<int,int> m;
    for (auto i = l.begin(); i != l.end(); i++) {
        m.insert(i->first, i->second);
    }
    return std::move(m);
}

typedef zedland::hash_map<int,int> hash_map_t;
typedef zedland::hash_set<hash_map_t,hash_hash_map> hash_set_t;

void test_hash_set_hash_map()
{
    hash_set_t s;
    s.insert(make_map({{1,2},{3,4},{5,6}}));
    s.insert(make_map({{1,2},{3,4},{5,6},{7,8}}));
    s.insert(make_map({{1,2},{3,4},{5,6},{7,8},{9,10}}));

    for (auto &h : s) {
        for (auto i = h.first.begin(); i != h.first.end(); i++) {
            if (i != h.first.begin()) printf(",");
            printf("%d=%d", i->first, i->second);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    test_hash_set_simple();
    test_hash_set_hash_map();
    return 0;
}