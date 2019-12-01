#include <cstdio>
#include <cstdint>
#include <cinttypes>

#include "hashmap.h"

static const char* words[] = {
	"melted",
	"frame",
	"combative",
	"panicky",
	"thing",
	"ruin",
	"fat",
	"damaged",
	"kick",
	"snail",
	"gainful",
	"literate",
	nullptr
};

template <typename HASH>
void test_hash_int()
{
	HASH hf;

	for (size_t i = 0; i < 12; i++) {
		printf("%20zu %016" PRIx64 "\n", i, hf(i));
	}
	printf("\n");
}

template <typename HASH>
void test_hash_str()
{
	HASH hf;

	const char** word = words;
	while (*word) {
		printf("%20s %016" PRIx64 "\n", *word, hf(*word));
		word++;
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	test_hash_str<hash_fnv>();
	test_hash_int<hash_fnv>();
	test_hash_int<hash_ident>();
	return 0;
}