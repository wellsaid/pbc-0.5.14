#if defined(CONTIKI_TARGET_ZOUL) && defined(ZOUL_USE_PKA)

#include <stdlib.h>

#include <ecc-driver.h>

#include "pbc_fp.h"
#include "pbc_pka.h"
#include "pbc_curve.h"

/* --------------------------------------------------- debug methods  ------------------------------------------------ */

//static void print_all_items(element_ptr x, const char* name){
//	mpz_t mpz_x_i;
//	element_ptr x_i;
//	char *p_str;
//	unsigned int p_str_size = 0;
//
//	mpz_init(mpz_x_i);
//	unsigned int item_count = element_item_count(x);
//	printf("%s = { ", name); fflush(stdout);
//	for(unsigned int i = 0; i < item_count; i++) {
//		x_i = element_item(x, i);
//		element_to_mpz(mpz_x_i, x_i);
//		p_str_size = mpz_sizeinbase(mpz_x_i, 10) + 1;
//		p_str = heapmem_alloc(p_str_size);
//		gmp_snprintf(p_str, p_str_size, "%Zd", mpz_x_i);
//		printf(p_str); fflush(stdout);
//		heapmem_free(p_str);
//
//		if(i < item_count - 1){
//			printf(", "); fflush(stdout);
//		}
//	}
//	printf(" }\n");
//}

//static void print_ecc_curve_info(ecc_curve_info_t curve, const char* label){
//	printf("%s = {\n", label);
//	printf("\t.size=%d\n", curve.size);
//
//	mpz_t prime;
//	mpz_init(prime);
//	limbs_array_to_mpz(prime, curve.prime, curve.size);
//
//	mpz_t a;
//	mpz_init(a);
//	limbs_array_to_mpz(a, curve.a, curve.size);
//
//	mpz_t b;
//	if(curve.b != NULL){
//		mpz_init(b);
//		limbs_array_to_mpz(b, curve.b, curve.size);
//	}
//
//	/* get largest parameter size in base 10 */
//	unsigned int max_size =
//			MAX( mpz_sizeinbase(a, 10),
//			MAX( (curve.b != NULL)?mpz_sizeinbase(b, 10):0,
//					mpz_sizeinbase(prime, 10)
//					 ));
//	unsigned int mpz_str_len = max_size + 1;
//	char mpz_str[mpz_str_len];
//
//	gmp_snprintf(mpz_str, mpz_str_len, "%Zd", prime);
//	printf("\t.prime=%s\n", mpz_str);
//
//	gmp_snprintf(mpz_str, mpz_str_len, "%Zd", a);
//	printf("\t.a=%s\n", mpz_str);
//
//	if(curve.b != NULL){
//		gmp_snprintf(mpz_str, mpz_str_len, "%Zd", b);
//		printf("\t.b=%s\n", mpz_str);
//	}
//
//	printf("}\n");
//}

/* ------------------------------------------------ (END) debug methods ---------------------------------------------- */

/* --------------------------------------------------- helper methods  ------------------------------------------------ */

/**
 * \brief converts an mpz to a fixed size limb array
 *
 * \param ls	The limb array (pre-alocated) which will be written
 * \param x	The mpz to convert
 * \param n	The size of limb array
 */
static void mpz_to_fixed_size_limbs_array(const uint32_t* ls, mpz_t x, size_t n){
	memset(ls, 0, n*sizeof(uint32_t));
	size_t countp;
	mpz_export(ls, &countp, -1, sizeof(uint32_t), -1, 0, x);
}

/**
 * \brief converts a limb array of fixed size into an mpz
 *
 * \param m	The mpz (pre-allocated and initialized) which will be written
 * \param ls	The limb array
 * \param n	The size of limb array
 */
static void limbs_array_to_mpz(mpz_t m, const uint32_t* ls, const size_t n){
	mpz_import(m, n, -1, sizeof(mp_limb_t), -1, 0, ls);
}

/**
 * \brief convert an ec_point_t into an element_t
 *
 * \param a					Pointer to the element_t to convert
 * \param curve_size		Size of each element in the curve
 * \return 						The generated ec_point_t
 */
static ec_point_t element_to_ec_point(element_ptr a, unsigned int curve_size){
	ec_point_t point_a;
	memset(&point_a, 0, sizeof(ec_point_t));

	element_ptr ptr_a_x = element_x(a);
	element_ptr ptr_a_y = element_y(a);

	mpz_t mpz_a_x;
	mpz_init(mpz_a_x);
	element_to_mpz(mpz_a_x, ptr_a_x);
	if(mpz_size(mpz_a_x) > curve_size){
		printf("ERROR: Element a too large\n");
		exit(1);
	}
	mpz_to_fixed_size_limbs_array(&point_a.x[0], mpz_a_x, 12);
	mpz_clear(mpz_a_x);

	mpz_t mpz_a_y;
	mpz_init(mpz_a_y);
	element_to_mpz(mpz_a_y, ptr_a_y);
	if(mpz_size(mpz_a_y) > curve_size){
		printf("ERROR: Element a too large\n");
		exit(1);
	}
	mpz_to_fixed_size_limbs_array(&point_a.y[0], mpz_a_y, 12);
	mpz_clear(mpz_a_y);

	return point_a;
}

/**
 * \brief convert an element_t into an ec_point_t
 *
 * \param c					Pointer to the element_t (pre-allocated) which will contain the result
 * \param point_c			The ec_point_t to convert
 */
static void ec_point_to_element(element_ptr c, ec_point_t point_c){
	element_ptr ptr_c_x = element_x(c);
	element_ptr ptr_c_y = element_y(c);

	mpz_t mpz_c_x;
	mpz_init(mpz_c_x);
	limbs_array_to_mpz(mpz_c_x, point_c.x, 12);
	element_set_mpz(ptr_c_x, mpz_c_x);
	mpz_clear(mpz_c_x);

	mpz_t mpz_c_y;
	mpz_init(mpz_c_y);
	limbs_array_to_mpz(mpz_c_y, point_c.y, 12);
	element_set_mpz(ptr_c_y, mpz_c_y);
	mpz_clear(mpz_c_y);
}

/**
 * \brief initialize structure for ecc operation specified
 *
 * \param curve		The curve structure to write
 * \param f			Pointer to the PBC field on which the operation will be performed
 * \parma op			The operation to prepare for (0 for sum, 1 for mul)
 */
static void init_ecc_operation(volatile ecc_curve_info_t curve, field_ptr f, const char op){
	if(curve.size > PKA_MAX_CURVE_SIZE){
		printf("ERROR: Curve too large\n");
		exit(1);
	}

	mpz_t mpz_a_coeff, mpz_b_coeff;

	/* getting all curve parameters from f */
	curve_data_ptr cdp = f->data;
	fptr fp = cdp->field->data;
	memcpy(curve.prime, fp->primelimbs, curve.size*sizeof(uint32_t));

	mpz_init(mpz_a_coeff);
	element_to_mpz(mpz_a_coeff, cdp->a);
	mpz_to_fixed_size_limbs_array(curve.a, mpz_a_coeff, curve.size);
	mpz_clear(mpz_a_coeff);

	if(op == 1){
		mpz_init(mpz_b_coeff);
		element_to_mpz(mpz_b_coeff, cdp->b);
		mpz_to_fixed_size_limbs_array(curve.b, mpz_b_coeff, curve.size);
		mpz_clear(mpz_b_coeff);
	}

	/* start pka engine */
	pka_enable();
}

/**
 * \brief correctly ends ecc operation
 */
static void finish_ecc_operation(void){
	/* stop pka engine */
	pka_disable();
}

/**
 * \brief Helper macro to correctly allocate an ecc_curve_info_t in static mem.
 */
#define STATIC_CURVE(curve, f)			\
		curve_data_ptr cdp = f->data;		\
		fptr fp = cdp->field->data;				\
		uint32_t a_coeff_buf[fp->limbs];		\
		uint32_t b_coeff_buf[fp->limbs];		\
		uint32_t prime[fp->limbs];				\
		ecc_curve_info_t curve = {				\
					.name = NULL,					\
					.size =  fp->limbs,				\
					.prime = prime,					\
					.n = NULL,						\
					.a = &a_coeff_buf[0],			\
					.b = &b_coeff_buf[0],			\
					.x = NULL,						\
					.y = NULL						\
			};												\

/* ---------------------------------------------- (END) helper methods  ------------------------------------------- */

/* --------------------------------------------------- pub. methods  ------------------------------------------------ */

void curve_add_pka(element_ptr c, element_ptr a, element_ptr b){
	/* initialize operation */
	STATIC_CURVE(curve, a->field);
	init_ecc_operation(curve, a->field, 0);

	uint32_t result_vec;

	ec_point_t point_a, point_b, point_c;
	memset(&point_c, 0, sizeof(ec_point_t));

	point_a = element_to_ec_point(a, curve.size);
	point_b = element_to_ec_point(b, curve.size);

	/* how to be notified when it has finished? -> You have to pass it a contiki process instead of NULL
	 * (TODO: How can i block the caller of THIS function)
	 */
	if( ecc_add_start(&point_a, &point_b, &curve, &result_vec, NULL) != PKA_STATUS_SUCCESS){
		printf("ERROR: starting ecc_add operation\n");
		exit(1);
	}

	/* TODO: make wait less ugly */
	uint8_t ret;
	while( (ret = ecc_add_get_result(&point_c, result_vec)) == PKA_STATUS_OPERATION_INPRG ); /* WARNING: very ugly way to wait! */

	if(ret == PKA_STATUS_FAILURE){
		printf("ERROR: getting result of ecc_add operation (err. %ld)\n", REG(PKA_SHIFT));
		exit(1);
	}

	/* put point_c in c */
	ec_point_to_element(c, point_c);
	((curve_point_ptr) c->data)->inf_flag = 0;

	/* finish operation */
	finish_ecc_operation();
}

void curve_mul_uint32_pka(element_ptr c, element_ptr a, uint32_t k){
	/* initialize operation */
	STATIC_CURVE(curve, a->field);
	init_ecc_operation(curve, a->field, 1);

	uint32_t result_vec;

	ec_point_t point_a, point_c;
	memset(&point_c, 0, sizeof(ec_point_t));

	point_a = element_to_ec_point(a, curve.size);
	uint32_t k_ls[curve.size];
	memset(&k_ls[0], 0, curve.size*sizeof(uint32_t));
	k_ls[0] = k;

	/* how to be notified when it has finished? -> You have to pass it a contiki process instead of NULL
	 * (TODO: How can i block the caller of THIS function)
	 */
	if( ecc_mul_start(k_ls, &point_a, &curve, &result_vec, NULL) != PKA_STATUS_SUCCESS){
		printf("ERROR: starting ecc_mul operation\n");
		exit(1);
	}

	/* TODO: make wait less ugly */
	uint8_t ret;
	while( (ret = ecc_mul_get_result(&point_c, result_vec)) == PKA_STATUS_OPERATION_INPRG ); /* WARNING: very ugly way to wait! */

	if(ret == PKA_STATUS_FAILURE){
		printf("ERROR: getting result of ecc_mul operation (err. %ld)\n", REG(PKA_SHIFT));
		exit(1);
	}

	/* put point_c in c */
	ec_point_to_element(c, point_c);
	((curve_point_ptr) c->data)->inf_flag = 0;

	/* finish operation */
	finish_ecc_operation();
}

void curve_mul_mpz_pka(element_ptr c, element_ptr a, mpz_ptr k){
	/* initialize operation */
	STATIC_CURVE(curve, a->field);
	init_ecc_operation(curve, a->field, 1);

	uint32_t result_vec;

	ec_point_t point_a, point_c;
	memset(&point_c, 0, sizeof(ec_point_t));

	point_a = element_to_ec_point(a, curve.size);
	uint32_t k_ls[curve.size];
	mpz_to_fixed_size_limbs_array(&k_ls[0], k, curve.size);

	/* how to be notified when it has finished? -> You have to pass it a contiki process instead of NULL
	 * (TODO: How can i block the caller of THIS function)
	 */
	if( ecc_mul_start(k_ls, &point_a, &curve, &result_vec, NULL) != PKA_STATUS_SUCCESS){
		printf("ERROR: starting ecc_mul operation\n");
		exit(1);
	}

	/* TODO: make wait less ugly */
	uint8_t ret;
	while( (ret = ecc_mul_get_result(&point_c, result_vec)) == PKA_STATUS_OPERATION_INPRG ); /* WARNING: very ugly way to wait! */

	if(ret == PKA_STATUS_FAILURE){
		printf("ERROR: getting result of ecc_mul operation (err. %ld)\n", REG(PKA_SHIFT));
		exit(1);
	}

	/* put point_c in c */
	ec_point_to_element(c, point_c);
	((curve_point_ptr) c->data)->inf_flag = 0;

	/* finish operation */
	finish_ecc_operation();
}

/* ---------------------------------------------- (END) pub. methods  ------------------------------------------- */

#endif
