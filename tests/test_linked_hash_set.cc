#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include <array>
#include <utility>
#include <initializer_list>

#include "sha256.h"
#include "linked_hash_map.h"
#include "linked_hash_set.h"

struct hash_linked_hash_map
{
    typedef ethical::linked_hash_map<int,int> hashmap_t;

    size_t operator()(hashmap_t &m)
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

void test_linked_hash_set_simple()
{
    static const uintptr_t numbers[] = { 8, 9, 6, 7, 4, 5, 2, 3, 0 };

    ethical::linked_hash_set<uintptr_t> ht;

    for (const uintptr_t *n = numbers; *n != 0; n++) {
        ht.insert(*n);
    }
    for (const uintptr_t *n = numbers; *n != 0; n++) {
        assert(ht.find(*n)->first == *n);
    }
    const uintptr_t *n = numbers;
    for (auto &ent : ht) {
        assert(ent.first == *n++);
    }
}

ethical::linked_hash_map<int,int>
make_map(std::initializer_list<std::pair<int,int>> l)
{
    ethical::linked_hash_map<int,int> m;
    for (auto i = l.begin(); i != l.end(); i++) {
        m.insert(i->first, i->second);
    }
    return std::move(m);
}

typedef ethical::linked_hash_map<int,int> hashmap_t;
typedef ethical::linked_hash_set<hashmap_t,int32_t,hash_linked_hash_map> hashset_t;

void test_linked_hash_set_hashmap()
{
    hashset_t s;
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
    test_linked_hash_set_simple();
    test_linked_hash_set_hashmap();
    return 0;
}