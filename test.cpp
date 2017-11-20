/**
 * `murmurhash.h' - murmurhash
 *
 * copyright (c) 2014 joseph werle <joseph.werle@gmail.com>
 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
// #include "murmurhash.h"

using namespace std;

int asciiHash(const char* key) {
    int hash = 0;
    for (int i = 0; i < (int) strlen(key); ++i ){      // calc ASCII value of key
      hash+= (int)key[i];
    }
  	return hash;
}

uint32_t murmurhash (const char *key, uint32_t len, uint32_t seed) {
  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;
  uint32_t r1 = 15;
  uint32_t r2 = 13;
  uint32_t m = 5;
  uint32_t n = 0xe6546b64;
  uint32_t h = 0;
  uint32_t k = 0;
  uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
  const uint32_t *chunks = NULL;
  const uint8_t *tail = NULL; // tail - last 8 bytes
  int i = 0;
  int l = len / 4; // chunk length
  h = seed;
  chunks = (const uint32_t *) (d + l * 4); // body
  tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'
  // for each 4 byte chunk of `key'
  for (i = -l; i != 0; ++i) {
    // next 4 byte chunk of `key'
    k = chunks[i];
    // encode next 4 byte chunk of `key'
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;
    // append to hash
    h ^= k;
    h = (h << r2) | (h >> (32 - r2));
    h = h * m + n;
  }
  k = 0;
  // remainder
  switch (len & 3) { // `len % 4'
    case 3: k ^= (tail[2] << 16);
    case 2: k ^= (tail[1] << 8);
    case 1:
      k ^= tail[0];
      k *= c1;
      k = (k << r1) | (k >> (32 - r1));
      k *= c2;
      h ^= k;
  }
  h ^= len;
  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h;
}

int main (void) {
  uint32_t seed = 0;
  const char *key = "jojo is jojocom";
  uint32_t hash = murmurhash(key, (uint32_t) strlen(key), seed); // 0xb6d99cf8

  // printf("%d\n", hash);
  bool bloom_array[10];
  for (int i = 0; i < 10; i++) {
      cout << "before " << bloom_array[i] << endl;
      bloom_array[i] = false;
      cout << "after " << bloom_array[i] << endl;
  }
  // bloom_array[hash] = true;
  // cout << bloom_array[hash] << endl;
  cout << "murmur3 " << hash << endl;
  cout << "ascii " << asciiHash(key) << endl;
  return 0;
}
