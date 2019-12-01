#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <chrono>
#include <random>
#include <map>
#include <vector>

#include "dense_hash_map"
#include "hashmap.h"

using namespace std::chrono;


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
	printf("|%-30s|%8s|%12zu|%8.1f|\n", buf, "random", count,
		duration_cast<nanoseconds>(t2-t1).count()/(float)count);
	snprintf(buf, sizeof(buf), "%s::lookup", name);
	printf("|%-30s|%8s|%12zu|%8.1f|\n", buf, "random", count,
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
	printf("|%-30s|%8zu|%12zu|%8.1f|\n", name, spread, count,
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
	printf("|%-30s|%8zu|%12zu|%8.1f|\n", name, spread, count,
		duration_cast<nanoseconds>(t2-t1).count()/(float)count);
}

void bench_hashmap_fnv(size_t count)
{
	hashmap<uintptr_t,uintptr_t,hash_fnv> ht;

	auto data = get_random(count);
	auto t1 = system_clock::now();
	for (auto &ent : data) {
    	ht.insert(ent.first, ent.second);
	}
	auto t2 = system_clock::now();
	for (auto &ent : data) {
		assert(ht.lookup(ent.first) == ent.second);
	}
	auto t3 = system_clock::now();

	print_timings("hashmap<FNV1amc>", t1, t2, t3, count);
}

void bench_hashmap_nop(size_t count)
{
	hashmap<uintptr_t,uintptr_t,hash_ident> ht;

	auto data = get_random(count);
	auto t1 = system_clock::now();
	for (auto &ent : data) {
    	ht.insert(ent.first, ent.second);
	}
	auto t2 = system_clock::now();
	for (auto &ent : data) {
		assert(ht.lookup(ent.first) == ent.second);
	}
	auto t3 = system_clock::now();

	print_timings("hashmap<ident>", t1, t2, t3, count);
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
	print_timings("dense_hash_map", t1, t2, t3, count);
}

void heading()
{
	printf("|%-30s|%8s|%12s|%8s|\n",
		"container", "spread", "count", "time_ns");
	printf("|%-30s|%8s|%12s|%8s|\n",
		":----------------------------", "-----:", "----:", "------:");
}

int main(int argc, char **argv)
{
	heading();
	bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,255);
	bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,1023);
	bench_generic<std::map<size_t,size_t>>("std::map::operator[]",10000000,16383);
	bench_stdmap(1<<20);
	bench_generic<hashmap<size_t,size_t,hash_fnv>>("hashmap<FNV1amc>::operator[]",10000000,255);
	bench_generic<hashmap<size_t,size_t,hash_fnv>>("hashmap<FNV1amc>::operator[]",10000000,1023);
	bench_generic<hashmap<size_t,size_t,hash_fnv>>("hashmap<FNV1amc>::operator[]",10000000,16383);
	bench_hashmap_fnv(1<<20);
	bench_generic<hashmap<size_t,size_t,hash_ident>>("hashmap<ident>::operator[]",10000000,255);
	bench_generic<hashmap<size_t,size_t,hash_ident>>("hashmap<ident>::operator[]",10000000,1023);
	bench_generic<hashmap<size_t,size_t,hash_ident>>("hashmap<ident>::operator[]",10000000,16383);
	bench_hashmap_nop(1<<20);
	bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,255);
	bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,1023);
	bench_google<google::dense_hash_map<uintptr_t,uintptr_t>>("dense_hash_map::operator[]",10000000,16383);
	bench_google_dense_hash_map(1<<20);
}