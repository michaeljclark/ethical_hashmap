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
    for (size_t i = 0; i < count; i++) {
        data.push_back(std::pair<K,V>(r.get<K>(), r.get<V>()));
    };
    return data;
}

template <typename T>
void print_timings(const char *name, T t1, T t2, T t3, size_t count)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%s::insert", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count);
    snprintf(buf, sizeof(buf), "%s::lookup", name);
    printf("|%-40s|%8s|%12zu|%8.1f|\n", buf, "random", count,
        duration_cast<nanoseconds>(t3-t2).count()/(float)count);
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
    printf("|%-40s|%8zu|%12zu|%8.1f|\n", name, spread, count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count);
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
    printf("|%-40s|%8zu|%12zu|%8.1f|\n", name, spread, count,
        duration_cast<nanoseconds>(t2-t1).count()/(float)count);
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
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t3 = system_clock::now();

    print_timings(name, t1, t2, t3, count);
}

template <typename Map>
void bench_map_google(const char* name, size_t count)
{
    typedef std::pair<typename Map::key_type,typename Map::mapped_type> pair_type;

    Map ht;
    ht.set_empty_key(-1);

    auto data = get_random<typename Map::key_type,typename Map::mapped_type>(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ht.end(), pair_type(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t3 = system_clock::now();

    print_timings(name, t1, t2, t3, count);
}

void heading()
{
    printf("benchmark: zedland::hashmap, dense_hash_map, std::unordered_map, std::map\n");
#ifndef _WIN32
    printf("cpu_model: %s\n", get_cpu_model().c_str());
#endif
    printf("\n");
    printf("|%-40s|%8s|%12s|%8s|\n",
        "container", "spread", "count", "time_ns");
    printf("|%-40s|%8s|%12s|%8s|\n",
        ":--------------------------------------", "-----:", "----:", "------:");
}

int main(int argc, char **argv)
{
    heading();
    bench_spread<std::map<size_t,size_t>>("std::map::operator[]",10000000,255);
    bench_spread<std::map<size_t,size_t>>("std::map::operator[]",10000000,1023);
    bench_spread<std::map<size_t,size_t>>("std::map::operator[]",10000000,16383);
    bench_map<std::map<size_t,size_t>>("std::map",1000000);
    bench_spread<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,255);
    bench_spread<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,1023);
    bench_spread<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,16383);
    bench_map<std::unordered_map<size_t,size_t>>("std::unordered_map", 1000000);
    bench_spread<zedland::hashmap<size_t,size_t>>("zedland::hashmap::operator[]",10000000,255);
    bench_spread<zedland::hashmap<size_t,size_t>>("zedland::hashmap::operator[]",10000000,1023);
    bench_spread<zedland::hashmap<size_t,size_t>>("zedland::hashmap::operator[]",10000000,16383);
    bench_map<zedland::hashmap<size_t,size_t>>("zedland::hashmap", 1000000);
    bench_spread_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map::operator[]",10000000,255);
    bench_spread_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map::operator[]",10000000,1023);
    bench_spread_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map::operator[]",10000000,16383);
    bench_map_google<google::dense_hash_map<size_t,size_t>>("google::dense_hash_map", 1000000);
    bench_spread<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map::operator[]",10000000,255);
    bench_spread<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map::operator[]",10000000,1023);
    bench_spread<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map::operator[]",10000000,16383);
    bench_map<absl::flat_hash_map<size_t,size_t>>("absl::flat_hash_map",1000000);
}