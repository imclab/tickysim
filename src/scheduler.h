/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * scheduler.h -- A scheduler, the generic heart of TickySim.
 *
 * The scheduler allows the scheduling of pairs of functions, named tick and
 * tock, to occur at a regular interval. At the period specified, the scheduler
 * will call all tick functions before then calling all tock functions.
 *
 * It is suggested that tick/tock pairs correspond to read and write phases of a
 * periodic process to ensure deterministic behaviour no-matter what order the
 * scheduler calls the events.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "config.h"


/**
 * The type of a time in ticks as used in the simulator.
 */
typedef unsigned int ticks_t;


/**
 * A structure which defines a particular instance of a scheduler.
 */
typedef struct scheduler scheduler_t;


// Concrete definitions of the above types
#include "scheduler_internal.h"


/**
 * Initialise a new scheduler instance struct.
 */
void scheduler_init(scheduler_t *scheduler);


/**
 * Free memory associated with a scheduler interface.
 */
void scheduler_destroy(scheduler_t *scheduler);


/**
 * Schedule a pair of functions to be called at a given period. The tick
 * function will be called during the first phase of the time period and the
 * tock in the second period (after all tick functions have been called).
 *
 * @param scheduler A pointer to the scheduler datastructure.
 * @param period The period of calls to the specified functions.
 * @param tick A function to be called with tick_data at the specified period.
 *             May be NULL to disable.
 * @param tick_data A void pointer to pass to tick. May be NULL.
 * @param tock A function to be called with tock_data at the specified period.
 *             May be NULL to disable.
 * @param tock_data A void pointer to pass to tock. May be NULL.
 */
void scheduler_schedule( scheduler_t *scheduler
                       , ticks_t period
                       , void (*tick)(void *)
                       , void *tick_data
                       , void (*tock)(void *)
                       , void *tock_data
                       );

/**
 * Get the current simulation time.
 */
ticks_t scheduler_get_ticks(scheduler_t *scheduler);

/**
 * Run the simulation for a single time-step.
 */
void scheduler_tick_tock(scheduler_t *scheduler);

#endif
