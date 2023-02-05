#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include <array>
#include <utility>
#include <initializer_list>

#include "bytes.h"
#include "sha256.h"
#include "linked_hash_map.h"


using ethical::linked_hash_map;

static char* hex_string(char *buf, size_t buf_len, const uint8_t *in, size_t len)
{
    size_t o = 0;
    for (size_t i = 0; i < len; i++) {
        o += snprintf(buf + o, buf_len - o, "%02" PRIX8, in[i]);
    }
    return buf;
}

template <typename H> static char* map_string(char *buf, size_t buf_len, H &map)
{
    size_t o = 0;
    for (auto i = map.begin(); i != map.end(); i++) {
        if (i != map.begin()) o += snprintf(buf + o, buf_len - o, ", ");
        o += snprintf(buf + o, buf_len - o, "%d:%d", i->first, i->second);
    }
    return buf;
}

typedef std::array<uint8_t,32> key256;
typedef linked_hash_map<int,int> pmap;

struct hash_key256 {
    size_t operator()(const key256 &b) {
        return le64(*(const uint64_t*)&b[0]);
    }
};

key256 pmap_key256(pmap &m)
{
    key256 b;
    sha256_ctx ctx;
    sha256_init(&ctx);
    for (auto &ent : m) {
        sha256_update(&ctx, (const void*)&ent.first, sizeof(ent.first));
        sha256_update(&ctx, (const void*)&ent.second, sizeof(ent.second));
    }
    sha256_final(&ctx, b.data());
    return b;
}

pmap make_map(std::initializer_list<std::pair<int,int>> l)
{
    pmap m;
    std::for_each(l.begin(), l.end(), [&](const auto &i) { m.insert(i); });
    return std::move(m);
}

typedef linked_hash_map<key256,pmap,int32_t,hash_key256> sha256_pmap_base;

struct sha256_pmap : sha256_pmap_base
{
    void insert(pmap &&m) { sha256_pmap_base::insert(pmap_key256(m), m); }
};

void test_hashmap_hashmap()
{
    sha256_pmap s;
    s.insert(make_map({{0,1},{1,4},{2,120},{3,60}}));
    s.insert(make_map({{0,2},{1,4},{2,240},{3,180}}));
    s.insert(make_map({{0,3},{1,4},{2,720},{3,360}}));
    s.insert(make_map({{0,4},{1,4},{2,1260},{3,840}}));
    s.insert(make_map({{0,5},{1,8},{2,2520},{3,1680}}));
    s.insert(make_map({{0,6},{1,8},{2,7560},{3,5040}}));
    s.insert(make_map({{0,7},{1,8},{2,15120},{3,10080}}));
    s.insert(make_map({{0,8},{1,8},{2,25200},{3,20160}}));
    s.insert(make_map({{0,9},{1,12},{2,45360},{3,27720}}));
    s.insert(make_map({{0,10},{1,12},{2,55440},{3,50400}}));
    s.insert(make_map({{0,11},{1,12},{2,110880},{3,83160}}));
    s.insert(make_map({{0,12},{1,12},{2,221760},{3,166320}}));
    s.insert(make_map({{0,13},{1,16},{2,332640},{3,277200}}));
    s.insert(make_map({{0,14},{1,16},{2,554400},{3,498960}}));
    s.insert(make_map({{0,15},{1,16},{2,720720},{3,665280}}));
    s.insert(make_map({{0,16},{1,16},{2,1441440},{3,1081080}}));

    char hs[128], ms[128];
    for (auto &h : s) {
        printf("%s → { %s }\n", hex_string(hs, sizeof(hs), &h.first[0], 16),
                                map_string(ms, sizeof(ms), h.second));
    }
}

/*
    starting an experiment with graph deltas to efficiently modify graph
    replicas in separate processes. like Git but finer-grained.
    a doubly-linked map of maps indexed by truncated SHA2 hash.
    a truncated content hash of the object used to address nodes.
*/

int main(int argc, char **argv)
{
    test_hashmap_hashmap();
    return 0;
}