#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <chrono>
#include <random>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include "hash_map.h"
#include "linked_hash_map.h"

using namespace std::chrono;

#ifndef _WIN32
std::string get_proc_info(const char *key)
{
    FILE *file;
    char buf[1024];
    char *line;

    if (!(file = fopen("/proc/cpuinfo", "r"))) {
        return "";
    }

    while ((line = fgets(buf, sizeof(buf), file)) != nullptr) {
        std::string s(line);
        if (s.find(key) != std::string::npos) {
            size_t c = s.find(":");
            return s.substr(c + 2, s.size() - c - 3);
        }
    }

    fclose(file);
    return "";
}

std::string get_cpu_model()
{
    std::string s = get_proc_info("model name");
    size_t c = s.find("@");
    return c != std::string::npos ? s.substr(0, c-1) : s;
}
#endif

struct rng
{
    std::random_device random_device;
    std::default_random_engine random_engine;
    std::uniform_int_distribution<uint64_t> random_dist;

    template <typename T>
    T get() { return random_dist(random_engine); }
};

template <typename K, typename V>
std::vector<std::pair<K,V>> get_random(size_t count)
{
    rng r;
    std::vector<std::pair<K,V>> data;
    ethical::hash_map<K,char> set;
    for (size_t i = 0; i < count; i++) {
        K key = r.get<K>();
        while (set[key] == 1) key = r.get<K>();
        V val = r.get<V>();
        data.push_back(std::pair<K,V>(key, val));
        set[key] = 1;
    };
    return data;
}

template <typename T>
void print_timings(const char *name, T t1, T t2, T t3, T t4, T t5, T t6, size_t count)
{
    char buf[128];
    snprintf(buf, sizeof(buf), "_%s::insert_", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count);
    snprintf(buf, sizeof(buf), "_%s::clear_", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t3-t2).count()/(float)count);
    snprintf(buf, sizeof(buf), "_%s::insert_", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t4-t3).count()/(float)count);
    snprintf(buf, sizeof(buf), "_%s::lookup_", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t5-t4).count()/(float)count);
    snprintf(buf, sizeof(buf), "_%s::erase_", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t6-t5).count()/(float)count);
    printf("|%-40s|%8s|%12s|%8s|\n", "-", "-", "-", "-");
}

static const size_t sizes[] = { 1023, 16383, 65535, 1048575, 0 };

template <typename Map>
void bench_spread(const char *name, size_t count, size_t spread)
{
    Map map;
    auto t1 = system_clock::now();
    for (size_t i = 0; i < count; i++) {
        map[i&spread]++;
    }
    auto t2 = system_clock::now();
    char buf[128];
    snprintf(buf, sizeof(buf), "_%s_", name);
    printf("|%-40s|%8zu|%12zu|%8.1f|\n", buf, spread, count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count);
}

template <typename Map>
void bench_spread(const char *name, size_t count)
{
    for (const size_t *s = sizes; *s != 0; s++) {
        bench_spread<Map>(name,count,*s);
    }
    printf("|%-40s|%8s|%12s|%8s|\n", "-", "-", "-", "-");
}

template <typename Map>
void bench_map(const char* name, size_t count)
{
    typedef std::pair<typename Map::key_type,typename Map::mapped_type> pair_type;

    Map ht;

    auto data = get_random<typename Map::key_type,typename Map::mapped_type>(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ht.end(), pair_type(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    ht.clear();
    auto t3 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ht.end(), pair_type(ent.first, ent.second));
    }
    auto t4 = system_clock::now();
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t5 = system_clock::now();
    for (auto &ent : data) {
        ht.erase(ent.first);
    }
    auto t6 = system_clock::now();

    print_timings(name, t1, t2, t3, t4, t5, t6, count);
}

void heading()
{
    printf("\n");
    printf("|%-40s|%8s|%12s|%8s|\n",
        "container", "spread", "count", "time_ns");
    printf("|%-40s|%8s|%12s|%8s|\n",
        ":--------------------------------------",
        "-----:", "----:", "------:");
}

int main(int argc, char **argv)
{
    size_t count = 1000000;

    if (argc == 2) {
        count = atoi(argv[1]);
    }

#ifndef _WIN32
    printf("cpu_model: %s\n", get_cpu_model().c_str());
#endif

    heading();
    bench_spread<ethical::hash_map<size_t,size_t>>("ethical::hash_map::operator[]",count);
    bench_spread<ethical::linked_hash_map<size_t,size_t>>("ethical::linked_hash_map::operator[]",count);

    heading();
    bench_map<ethical::hash_map<size_t,size_t>>("ethical::hash_map", count);
    bench_map<ethical::linked_hash_map<size_t,size_t>>("ethical::linked_hash_map", count);
}