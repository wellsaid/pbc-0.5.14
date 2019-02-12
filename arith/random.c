#include <stdio.h>
#include <stdint.h> // for intptr_t
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#include <os/lib/random.h>

#include "pbc_random.h"
#include "pbc_utils.h"
#include "pbc_memory.h"

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
