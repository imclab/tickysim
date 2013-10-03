/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_sim.h -- SpiNNaker simulation data structures for simulating a whole
 * system.
 */

#ifndef SPINN_SIM_H
#define SPINN_SIM_H

#include <libconfig.h>
#include <stdio.h>

#include "scheduler.h"
#include "buffer.h"
#include "arbiter.h"
#include "delay.h"

#include "spinn.h"
#include "spinn_packet.h"
#include "spinn_router.h"

typedef struct spinn_sim spinn_sim_t;
typedef struct spinn_node spinn_node_t;

/**
 * Resources for a single node in the simulation.
 */
struct spinn_node {
	// Back-reference to the simulation
	spinn_sim_t *sim;
	
	// The router
	spinn_router_t router;
	
	// The position of the node in the system
	spinn_coord_t position;
	
	// The source/sink for packets
	spinn_packet_gen_t packet_gen;
	spinn_packet_con_t packet_con;
	
	// The arbiters
	arbiter_t arb_n_s;
	arbiter_t arb_e_w;
	arbiter_t arb_ne_sw;
	
	arbiter_t arb_n_s_e_w;
	arbiter_t arb_ne_sw_l;
	
	arbiter_t arb_last;
	
	// Buffers on either side of a delay model which (eventually, via some
	// arbiters) connect to the inputs of the router
	buffer_t input_buffers[6];
	buffer_t output_buffers[6];
	
	// Delay element which connects each output to an input of a surrounding node
	// (connected up by spinn_sim_init).
	delay_t delays[6];
	
	// Packet generator/consumer buffers
	buffer_t gen_buffer;
	buffer_t con_buffer;
	
	// Buffers in the arbiter tree
	buffer_t arb_n_s_out;
	buffer_t arb_e_w_out;
	buffer_t arb_ne_sw_out;
	
	buffer_t arb_n_s_e_w_out;
	buffer_t arb_ne_sw_l_out;
	
	buffer_t arb_last_out;
	
	// Stat counters
	int stat_packets_offered;
	int stat_packets_accepted;
	int stat_packets_arrived;
	int stat_packets_dropped;
};


/**
 * Resources used by a SpiNNaker system simulation.
 */
struct spinn_sim {
	// Configuration file dictating the simulation parameters
	config_t config;
	
	// Scheduler which runs the simulation
	scheduler_t scheduler;
	
	// Packet memory allocation
	spinn_packet_pool_t pool;
	
	// An array of all of the spinnaker nodes
	spinn_node_t *nodes;
	
	// The size of the simulation
	spinn_coord_t system_size;
	
	// Statistic output files
	FILE *stat_file_global_counters;
	FILE *stat_file_per_node_counters;
	FILE *stat_file_packet_details;
	
	// Flags as to whether packet arrivals will be monitored
	bool stat_log_delivered_packets;
	bool stat_log_dropped_packets;
	
	// The time of the scheduler when the result collection started
	ticks_t stat_start_ticks;
	
	// Is the stat recording process running
	bool stat_started;
	
	// The experemental group currently being run
	int cur_group;
	
	// The sample currently being run
	int cur_sample;
	
	// The independent variables (and their names)
	int                num_ivars;
	config_setting_t **ivar_settings;
	char        const**ivar_names;
};



/**
 * Initialise a model for simulation based on the configuration stored in the
 * file named by config_filename.
 *
 * Also takes a set of arguments for the config file overrides.
 */
void spinn_sim_init(spinn_sim_t *sim, const char *config_filename, int argc, char *argv[]);


/**
 * Run the given simulation to completion.
 */
void spinn_sim_run(spinn_sim_t *sim);


/**
 * Free up resources used by a simulation.
 */
void spinn_sim_destroy(spinn_sim_t *sim);

#endif
