#include <stdio.h>
#include <stdint.h> // for intptr_t
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#include <os/lib/random.h>
#include <os/lib/heapmem.h>

#include "pbc_utils.h"
#include "pbc_random.h"

void contiki_randfunc(mpz_t num, mpz_t limit, void* data){
	uint32_t low = 0;
	low += random_rand();
	uint32_t high = 0;
	high += random_rand();
	uint32_t n = low + (high<<sizeof(unsigned short));
	mpz_init_set_ui(num, n);
}

void pbc_init_random(void) {
	random_init(time(NULL));
	pbc_random_set_function(contiki_randfunc, NULL);
}

