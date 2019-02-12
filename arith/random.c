#include <stdio.h>
#include <stdint.h> // for intptr_t
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#include <os/lib/random.h>

#include "pbc_random.h"
#include "pbc_utils.h"
#include "pbc_memory.h"

// Must use pointer due to lack of gmp_randstate_ptr.
static gmp_randstate_t *get_rs(void) {
  static int rs_is_ready;
  static gmp_randstate_t rs;
  if (!rs_is_ready) {
    gmp_randinit_default(rs);
    rs_is_ready = 1;
  }
  return &rs;
}

static void deterministic_mpz_random(mpz_t z, mpz_t limit, void *data) {
  UNUSED_VAR (data);
  mpz_urandomm(z, *get_rs(), limit);
}

static void file_mpz_random(mpz_t r, mpz_t limit, void *data) {
  char *filename = (char *) data;
  FILE *fp;
  int n, bytecount, leftover;
  unsigned char *bytes;
  mpz_t z;
  mpz_init(z);
  fp = fopen(filename, "rb");
  if (!fp) return;
  n = mpz_sizeinbase(limit, 2);
  bytecount = (n + 7) / 8;
  leftover = n % 8;
  bytes = (unsigned char *) pbc_malloc(bytecount);
  for (;;) {
    if (!fread(bytes, 1, bytecount, fp)) {
      pbc_warn("error reading source of random bits");
      return;
    }
    if (leftover) {
      *bytes = *bytes % (1 << leftover);
    }
    mpz_import(z, bytecount, 1, 1, 0, 0, bytes);
    if (mpz_cmp(z, limit) < 0) break;
  }
  fclose(fp);
  mpz_set(r, z);
  mpz_clear(z);
  pbc_free(bytes);
}

void pbc_mpz_random(mpz_t z, mpz_t limit) {
	UNUSED_VAR(limit);

	/* Impose limit? In ESP32 implementation i did not do it! */
	uint32_t low = 0;
	low += random_rand();
	uint32_t high = 0;
	high += random_rand();
	uint32_t n = low + (high<<sizeof(unsigned short));
	mpz_init_set_ui(z, n);
}

void pbc_mpz_randomb(mpz_t z, unsigned int bits) {
  mpz_t limit;
  mpz_init(limit);
  mpz_setbit(limit, bits);
  pbc_mpz_random(z, limit);
  mpz_clear(limit);
}

void pbc_random_set_deterministic(unsigned int seed) {
  gmp_randseed_ui(*get_rs(), seed);
  pbc_random_set_function(deterministic_mpz_random, NULL);
}

void pbc_random_set_file(char *filename) {
  pbc_random_set_function(file_mpz_random, filename);
}
