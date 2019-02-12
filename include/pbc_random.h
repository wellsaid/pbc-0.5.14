// Requires:
// * gmp.h
#ifndef __PBC_RANDOM_H__
#define __PBC_RANDOM_H__

/*@manual pbcrandom
Initializes the pbc random moduile
 */
void pbc_random_init(void);

/*@manual pbcrandom
Selects a random 'z' that is less than 'limit'.
*/
void pbc_mpz_random(mpz_t z, mpz_t limit);

/*@manual pbcrandom
Selects a random 'bits'-bit integer 'z'.
*/
void pbc_mpz_randomb(mpz_t z, unsigned int bits);

#endif //__PBC_RANDOM_H__
