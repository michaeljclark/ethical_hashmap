#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <chrono>
#include <random>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

#include "absl/hash/hash.h"
#include "absl/container/flat_hash_map.h"

#include "tsl/robin_map.h"

#include "dense_hash_map"
#include "hashmap.h"

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
    zedland::hashmap<K,char> set;
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
void print_timings(const char *name, T t1, T t2, T t3, T t4, T t5, T t6, T t7, size_t count)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%s::insert", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count,
        duration_cast<nanoseconds>(t2-t1).count()/1e9);
    snprintf(buf, sizeof(buf), "%s::clear", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t3-t2).count()/(float)count,
        duration_cast<nanoseconds>(t3-t2).count()/1e9);
    snprintf(buf, sizeof(buf), "%s::insert", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t4-t3).count()/(float)count,
        duration_cast<nanoseconds>(t4-t3).count()/1e9);
    snprintf(buf, sizeof(buf), "%s::erase", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t5-t4).count()/(float)count,
        duration_cast<nanoseconds>(t5-t4).count()/1e9);
    snprintf(buf, sizeof(buf), "%s::insert", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t6-t5).count()/(float)count,
        duration_cast<nanoseconds>(t6-t5).count()/1e9);
    snprintf(buf, sizeof(buf), "%s::lookup", name);
    printf("|%-40s|%8s|%12zu|%8.1f|%8.3f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t7-t6).count()/(float)count,
        duration_cast<nanoseconds>(t7-t6).count()/1e9);
    printf("|%71s|%8.3f|\n", "(insert,clear,insert,erase)",
        duration_cast<nanoseconds>(t5-t1).count()/1e9);
    printf("|%71s|%8.3f|\n", "(insert,clear,insert,erase,insert,lookup)",
        duration_cast<nanoseconds>(t7-t1).count()/1e9);
}

template <typename Map>
void bench_spread(const char *name, size_t count, size_t spread)
{
    Map map;
    auto t1 = system_clock::now();
    for (size_t i = 0; i < count; i++) {
        map[i&spread]++;
    }
    auto t2 = system_clock::now();
    printf("|%-40s|%8zu|%12zu|%8.1f|%8.3f|\n", name, spread, count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count,
        duration_cast<nanoseconds>(t2-t1).count()/1e9);
}

template <typename Map>
void bench_spread_google(const char *name, size_t count, size_t spread)
{
    Map map;
    map.set_empty_key(-1);
    auto t1 = system_clock::now();
    for (size_t i = 0; i < count; i++) {
        map[i&spread]++;
    }
    auto t2 = system_clock::now();
    printf("|%-40s|%8zu|%12zu|%8.1f|%8.3f|\n", name, spread, count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count,
        duration_cast<nanoseconds>(t2-t1).count()/1e9);
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
        ht.erase(ent.first);
    }
    auto t5 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ht.end(), pair_type(ent.first, ent.second));
    }
    auto t6 = system_clock::now();
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t7 = system_clock::now();

    print_timings(name, t1, t2, t3, t4, t5, t6, t7, count);
}

template <typename Map>
void bench_map_google(const char* name, size_t count)
{
    typedef std::pair<typename Map::key_type,typename Map::mapped_type> pair_type;

    Map ht;
    ht.set_empty_key(-1);
    ht.set_deleted_key(-2);

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
        ht.erase(ent.first);
    }
    auto t5 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ht.end(), pair_type(ent.first, ent.second));
    }
    auto t6 = system_clock::now();
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t7 = system_clock::now();

    print_timings(name, t1, t2, t3, t4, t5, t6, t7, count);
}

void heading()
{
    printf("\n");
    printf("|%-40s|%8s|%12s|%8s|%8s|\n",
        "container", "spread", "count", "time_ns", "duration");
    printf("|%-40s|%8s|%12s|%8s|%8s|\n",
        ":--------------------------------------", "-----:", "----:", "------:", "------:");
}

int main(int argc, char **argv)
{
    size_t count = 1000000;
    size_t sizes[] = { 1023, 16383, 65535, 1048575, 0 };

    if (argc == 2) {
        count = atoi(argv[1]);
    }

    printf("benchmark: tsl::robin_map, zedland::hashmap, google::dense_hash_map, absl::flat_hash_map\n");
#ifndef _WIN32
    printf("cpu_model: %s\n", get_cpu_model().c_str());
#endif

    for (size_t *s = sizes; *s != 0; s++) {
        heading();
        bench_spread<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",count,*s);
        bench_spread<tsl::robin_map<size_t,size_t>>("tsl::robin_map::operator[]",count,*s);
        bench_spread<zedland::hashmap<size_t,size_t>>("zedland::hashmap::operator[]",count,*s);
        bench_spread_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map::operator[]",count,*s);
        bench_spread<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map::operator[]",count,*s);
    }

    heading();
    bench_map<std::unordered_map<size_t,size_t>>("std::unordered_map", count);
    bench_map<tsl::robin_map<size_t,size_t>>("tsl::robin_map", count);
    bench_map<zedland::hashmap<size_t,size_t>>("zedland::hashmap", count);
    bench_map_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map", count);
    bench_map<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map",count);
}