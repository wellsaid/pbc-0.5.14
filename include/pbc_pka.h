#ifndef PBC_PKA_PBC_PKA_H_
#define PBC_PKA_PBC_PKA_H_

#include <gmp.h>

#include "pbc_field.h"

/**
 * \brief c = a + b
 *
 * WARNING: pka_init() must be called one time before this methods
 *
 * \param c
 * \param b
 * \param a
 */
void curve_add_pka(element_ptr c, element_ptr a, element_ptr b);

/**
 * \brief c = k*a
 *
 * WARNING: pka_init() must be called one time before this methods
 *
 * \param c
 * \param k
 * \param a
 */
void curve_mul_uint32_pka(element_ptr c, element_ptr a, uint32_t k);

/**
 * \brief c = k*a
 *
 * WARNING: pka_init() must be called one time before this methods
 *
 * \param c
 * \param k
 * \param a
 */
void curve_mul_mpz_pka(element_ptr c, element_ptr a, mpz_t k);

#endif /* PBC_PKA_PBC_PKA_H_ */
