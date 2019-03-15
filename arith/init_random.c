#include <stdio.h>
#include <stdint.h> // for intptr_t
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

#include <os/lib/random.h>
//#include <os/lib/heapmem.h>

#include "pbc_utils.h"
#include "pbc_random.h"

void pbc_random_init() {
	random_init(time(NULL));
}
