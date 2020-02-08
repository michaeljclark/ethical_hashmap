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

std::vector<std::pair<uintptr_t,uintptr_t>> get_random(size_t count)
{
    std::vector<std::pair<uintptr_t,uintptr_t>> data;
    insert_random(count, [&](size_t key, size_t val) {
        data.push_back(std::pair<uintptr_t,uintptr_t>(key, val));
    });
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
void bench_generic(const char *name, size_t count, size_t spread)
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
void bench_google(const char *name, size_t count, size_t spread)
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

template <typename HASH>
void bench_zhashmap(const char* name, size_t count)
{
    zhashmap<uintptr_t,uintptr_t,HASH> ht;

    auto data = get_random(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        ht.insert(ent.first, ent.second);
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(ht.find(ent.first)->second == ent.second);
    }
    auto t3 = system_clock::now();

    print_timings(name, t1, t2, t3, count);
}

void bench_stdmap(size_t count)
{
    std::map<uintptr_t,uintptr_t> hm;

    auto data = get_random(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        hm.insert(hm.end(), std::pair<uintptr_t,uintptr_t>(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(hm[ent.first] == ent.second);
    }
    auto t3 = system_clock::now();

    print_timings("std::map", t1, t2, t3, count);
}

void bench_unmap(size_t count)
{
    std::unordered_map<uintptr_t,uintptr_t> hm;

    auto data = get_random(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        hm.insert(hm.end(), std::pair<uintptr_t,uintptr_t>(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(hm[ent.first] == ent.second);
    }
    auto t3 = system_clock::now();

    print_timings("std::unordered_map", t1, t2, t3, count);
}

void bench_google_dense_hash_map(size_t count)
{
    google::dense_hash_map<uintptr_t,uintptr_t> hm;
    hm.set_empty_key(-1);

    auto data = get_random(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        hm.insert(hm.end(), std::pair<uintptr_t,uintptr_t>(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(hm[ent.first] == ent.second);
    }
    auto t3 = system_clock::now();
    print_timings("google::dense_hash_map", t1, t2, t3, count);
}

void bench_absl_flat_hash_map(size_t count)
{
    absl::flat_hash_map<uintptr_t,uintptr_t> hm;

    auto data = get_random(count);
    auto t1 = system_clock::now();
    for (auto &ent : data) {
        hm.insert(hm.end(), std::pair<uintptr_t,uintptr_t>(ent.first, ent.second));
    }
    auto t2 = system_clock::now();
    for (auto &ent : data) {
        assert(hm[ent.first] == ent.second);
    }
    auto t3 = system_clock::now();
    print_timings("absl::flat_hash_map", t1, t2, t3, count);
}

void heading()
{
    printf("benchmark: zhashmap, dense_hash_map, std::unordered_map, std::map\n");
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
    bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,255);
    bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,1023);
    bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,16383);
    bench_stdmap(1000000);
    bench_generic<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,255);
    bench_generic<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,1023);
    bench_generic<std::unordered_map<size_t,size_t>>("std::unordered_map::operator[]",10000000,16383);
    bench_unmap(1000000);
    bench_generic<zhashmap<size_t,size_t,std::hash<uintptr_t>>>("zhashmap<std::hash>::operator[]",10000000,255);
    bench_generic<zhashmap<size_t,size_t,std::hash<uintptr_t>>>("zhashmap<std::hash>::operator[]",10000000,1023);
    bench_generic<zhashmap<size_t,size_t,std::hash<uintptr_t>>>("zhashmap<std::hash>::operator[]",10000000,16383);
    bench_zhashmap<std::hash<uintptr_t>>("zhashmap<std::hash>", 1000000);
    bench_generic<zhashmap<size_t,size_t,hash_ident>>("zhashmap<hash_ident>::operator[]",10000000,255);
    bench_generic<zhashmap<size_t,size_t,hash_ident>>("zhashmap<hash_ident>::operator[]",10000000,1023);
    bench_generic<zhashmap<size_t,size_t,hash_ident>>("zhashmap<hash_ident>::operator[]",10000000,16383);
    bench_zhashmap<hash_ident>("zhashmap<hash_ident>", 1000000);
    bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,255);
    bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,1023);
    bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,16383);
    bench_google_dense_hash_map(1000000);
    bench_generic<absl::flat_hash_map<uintptr_t,uintptr_t>>("absl::flat_hash_map::operator[]",10000000,255);
    bench_generic<absl::flat_hash_map<uintptr_t,uintptr_t>>("absl::flat_hash_map::operator[]",10000000,1023);
    bench_generic<absl::flat_hash_map<uintptr_t,uintptr_t>>("absl::flat_hash_map::operator[]",10000000,16383);
    bench_absl_flat_hash_map(1000000);
}