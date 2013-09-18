/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * check_topology.c -- Unit tests for topology utility functions.
 */

#include <check.h>

#include "config.h"

#include "check_check.h"
#include "../src/spinn.h"
#include "../src/spinn_topology.h"

START_TEST (test_next_ccw)
{
	ck_assert_int_eq(spinn_next_ccw(SPINN_EAST),       SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_NORTH_EAST), SPINN_NORTH);
	ck_assert_int_eq(spinn_next_ccw(SPINN_NORTH),      SPINN_WEST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_WEST),       SPINN_SOUTH_WEST);
	ck_assert_int_eq(spinn_next_ccw(SPINN_SOUTH_WEST), SPINN_SOUTH);
	ck_assert_int_eq(spinn_next_ccw(SPINN_SOUTH),      SPINN_EAST);
}
END_TEST

START_TEST (test_next_cw)
{
	ck_assert_int_eq(spinn_next_cw(SPINN_EAST),       SPINN_SOUTH);
	ck_assert_int_eq(spinn_next_cw(SPINN_NORTH_EAST), SPINN_EAST);
	ck_assert_int_eq(spinn_next_cw(SPINN_NORTH),      SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_next_cw(SPINN_WEST),       SPINN_NORTH);
	ck_assert_int_eq(spinn_next_cw(SPINN_SOUTH_WEST), SPINN_WEST);
	ck_assert_int_eq(spinn_next_cw(SPINN_SOUTH),      SPINN_SOUTH_WEST);
}
END_TEST

START_TEST (test_opposite)
{
	ck_assert_int_eq(spinn_opposite(SPINN_EAST),       SPINN_WEST);
	ck_assert_int_eq(spinn_opposite(SPINN_NORTH_EAST), SPINN_SOUTH_WEST);
	ck_assert_int_eq(spinn_opposite(SPINN_NORTH),      SPINN_SOUTH);
	ck_assert_int_eq(spinn_opposite(SPINN_WEST),       SPINN_EAST);
	ck_assert_int_eq(spinn_opposite(SPINN_SOUTH_WEST), SPINN_NORTH_EAST);
	ck_assert_int_eq(spinn_opposite(SPINN_SOUTH),      SPINN_NORTH);
}
END_TEST

// Element-wise compare of a set of coords.
#define ck_assert_full_coord_eq(a,b) do { ck_assert_int_eq((a).x, (b).x); \
                                          ck_assert_int_eq((a).y, (b).y); \
                                          ck_assert_int_eq((a).z, (b).z); \
                                        } while (0)

// Check that a full coordinate minimised by spinn_full_coord_minimise equals
// the given value.
#define ck_assert_full_coord_minimise_eq(ax,ay,az, bx,by,bz) \
	do { \
		spinn_full_coord_t c = spinn_full_coord_minimise((spinn_full_coord_t){ax,ay,az}); \
		ck_assert_full_coord_eq(c, ((spinn_full_coord_t){bx,by,bz})); \
	} while (0)

START_TEST (test_full_coord_minimise)
{
	// Converts all-equal tuples to zeros
	ck_assert_full_coord_minimise_eq( 0, 0, 0,    0, 0, 0);
	ck_assert_full_coord_minimise_eq( 1, 1, 1,    0, 0, 0);
	ck_assert_full_coord_minimise_eq(-1,-1,-1,    0, 0, 0);
	
	// Leaves all one-non-zero values alone
	ck_assert_full_coord_minimise_eq(-1, 0, 0,   -1, 0, 0);
	ck_assert_full_coord_minimise_eq( 1, 0, 0,    1, 0, 0);
	ck_assert_full_coord_minimise_eq( 0,-1, 0,    0,-1, 0);
	ck_assert_full_coord_minimise_eq( 0, 1, 0,    0, 1, 0);
	ck_assert_full_coord_minimise_eq( 0, 0,-1,    0, 0,-1);
	ck_assert_full_coord_minimise_eq( 0, 0, 1,    0, 0, 1);
	
	// Pairs of values the same get minimised
	ck_assert_full_coord_minimise_eq(-1,-1, 0,    0, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 1, 0,    0, 0,-1);
	ck_assert_full_coord_minimise_eq(-1, 0,-1,    0, 1, 0);
	ck_assert_full_coord_minimise_eq( 1, 0, 1,    0,-1, 0);
	ck_assert_full_coord_minimise_eq( 0,-1,-1,    1, 0, 0);
	ck_assert_full_coord_minimise_eq( 0, 1, 1,   -1, 0, 0);
	
	// With no zeros things still get optimised
	ck_assert_full_coord_minimise_eq( 3, 2,  1,    1, 0,-1);
	ck_assert_full_coord_minimise_eq( 1, 2,  3,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 3,  2,   -1, 1, 0);
	ck_assert_full_coord_minimise_eq( 3, 1,  2,    1,-1, 0);
	ck_assert_full_coord_minimise_eq( 2, 3,  1,    0, 1,-1);
	ck_assert_full_coord_minimise_eq( 2, 1,  3,    0,-1, 1);
	ck_assert_full_coord_minimise_eq(-3,-2, -1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq(-1,-2, -3,    1, 0,-1);
	ck_assert_full_coord_minimise_eq(-1,-3, -2,    1,-1, 0);
	ck_assert_full_coord_minimise_eq(-3,-1, -2,   -1, 1, 0);
	ck_assert_full_coord_minimise_eq(-2,-3, -1,    0,-1, 1);
	ck_assert_full_coord_minimise_eq(-2,-1, -3,    0, 1,-1);
	
	// Already optimised things get left as they are
	ck_assert_full_coord_minimise_eq(-1, 0, 1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 0,-1,    1, 0,-1);
	ck_assert_full_coord_minimise_eq( 0,-1, 1,    0,-1, 1);
	ck_assert_full_coord_minimise_eq( 0, 1,-1,    0, 1,-1);
	ck_assert_full_coord_minimise_eq(-1, 0, 1,   -1, 0, 1);
	ck_assert_full_coord_minimise_eq( 1, 0,-1,    1, 0,-1);
}
END_TEST


Suite *
make_spinn_topology_suite(void)
{
	Suite *s = suite_create("spinn_topology");
	
	// Add tests to the test case
	TCase *tc_core = tcase_create("Core");
	tcase_add_test(tc_core, test_next_ccw);
	tcase_add_test(tc_core, test_next_cw);
	tcase_add_test(tc_core, test_opposite);
	tcase_add_test(tc_core, test_full_coord_minimise);
	
	// Add each test case to the suite
	suite_add_tcase(s, tc_core);
	
	return s;
}


