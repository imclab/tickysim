/**
 * TickySim -- A timing based interconnection network simulator.
 *
 * spinn_router.h -- A SpiNNaker router model.
 */


#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "scheduler.h"
#include "buffer.h"

#include "spinn.h"
#include "spinn_topology.h"
#include "spinn_packet.h"
#include "spinn_router.h"

/******************************************************************************
 * Internal datastructures.
 ******************************************************************************/

/**
 * The structure representing a particular router. 
 */
struct spinn_router {
	// Ports (expected to supply/accept spinn_packet_t pointers.
	buffer_t *input;
	buffer_t *outputs[7];
	
	// Location of the router in the system
	spinn_coord_t position;
	
	// Enable emergency routing (rather than just dropping out after
	// first_timeout.
	bool use_emg_routing;
	
	// Timeouts (measured in router periods)
	int first_timeout;
	int final_timeout;
	
	// Packet-forwarded callback
	void (*on_forward)( spinn_router_t    *router
	                  , spinn_packet_t    *packet
	                  , void              *data
	                  );
	void *on_forward_data;
	
	// Packet-droping callback
	void (*on_drop)( spinn_router_t *router
	               , spinn_packet_t *packet
	               , void           *data
	               );
	void *on_drop_data;
	
	// Number of cycles the current packet at the head of the input buffer has
	// been waiting to be routed (in router cycles).
	int time_elapsed;
	
	// The direction the next packet forwarded is being sent in (for the setting
	// of the flag in the packet.
	spinn_direction_t selected_output_direction;
	
	// The emergency routing state to be assigned to the packet when it is
	// forwarded
	spinn_emg_state_t cur_packet_emg_state;
	
	// Should the currrent packet be forwarded in the next tock?
	bool forward_packet;
	
	// Should the currrent packet should be dropped in the next tock?
	bool drop_packet;
};


/******************************************************************************
 * Internal functions.
 ******************************************************************************/

/**
 * Work out which port the packet should be sent from, assuming that it is being
 * routed normally.
 */
spinn_direction_t
get_packet_output_direction(spinn_router_t *r, spinn_packet_t *p)
{
	// Except at the inflection point and endpoint, just keep moving in the same
	// direction
	if ( r->position.x == p->destination.x &&
	     r->position.y == p->destination.y)
		return SPINN_LOCAL;
	else if ( r->position.x == p->inflection_point.x &&
	          r->position.y == p->inflection_point.y)
		return p->inflection_direction;
	else if (p->emg_state == SPINN_EMG_SECOND_LEG)
		return spinn_next_cw(p->direction);
	else
		return p->direction;
}


void
spinn_router_tick(void *r_)
{
	spinn_router_t *r = (spinn_router_t *)r_;
	
	// If there no packet to route, give up
	if (buffer_is_empty(r->input))
		return;
	
	spinn_packet_t *p = buffer_peek(r->input);
	
	// Find out the intended direction and emergency mode of the packet
	switch (p->emg_state) {
		case SPINN_EMG_NORMAL:
		case SPINN_EMG_SECOND_LEG:
			r->selected_output_direction = get_packet_output_direction(r, p);
			if (r->use_emg_routing && r->time_elapsed >= r->first_timeout) {
				r->cur_packet_emg_state      = SPINN_EMG_FIRST_LEG;
				r->selected_output_direction = spinn_next_cw(r->selected_output_direction);
			} else {
				r->cur_packet_emg_state = SPINN_EMG_NORMAL;
			}
			break;
		
		case SPINN_EMG_FIRST_LEG:
			r->cur_packet_emg_state = SPINN_EMG_SECOND_LEG;
			r->selected_output_direction = spinn_next_cw(spinn_opposite(p->direction));
			break;
	}
	
	if (!buffer_is_full(r->outputs[r->selected_output_direction])) {
		// Is the output available? Forward the packet to this port!
		r->forward_packet = true;
	} else if (!r->use_emg_routing &&
	           p->emg_state != SPINN_EMG_FIRST_LEG &&
	           r->time_elapsed >= r->first_timeout) {
		// Drop the packet as emergency routing is disabled and it has timed out
		r->drop_packet = true;
	} else if ( (p->emg_state == SPINN_EMG_FIRST_LEG &&
	             r->time_elapsed > r->first_timeout) ||
	           r->time_elapsed >= r->first_timeout + r->final_timeout){
		// TODO: Drop the timed-out emergency routed packet
		r->drop_packet = true;
	}
}


void
spinn_router_tock(void *r_)
{
	spinn_router_t *r = (spinn_router_t *)r_;
	
	// If there no packet to be sent, nothing to do so give up now!
	if (buffer_is_empty(r->input)) {
		return;
	} else if (!r->forward_packet && !r->drop_packet) {
		// If no forwarding/dropping to do, just advance the clock and stop
		r->time_elapsed++;
		return;
	} else {
		// Grab the packet from the input buffer
		spinn_packet_t *p = buffer_pop(r->input);
		
		if (r->forward_packet) {
			// Set the packet flags
			p->direction = r->selected_output_direction;
			p->emg_state = r->cur_packet_emg_state;
			
			// Forward the current packet to the output
			buffer_push(r->outputs[r->selected_output_direction], p);
			
			// Raise the forwarding callback
			if (r->on_forward != NULL)
				r->on_forward(r, p, r->on_forward_data);
		} else if (r->drop_packet && r->on_drop != NULL) {
			// Drop the packet at the input
			r->on_drop(r, p, r->on_drop_data);
		}
		
		// Reset router state ready for next packet
		r->drop_packet = false;
		r->forward_packet = false;
		r->time_elapsed = 0;
	}
}


/******************************************************************************
 * Public functions.
 ******************************************************************************/

spinn_router_t *
spinn_router_create( scheduler_t   *s
                   , ticks_t        period
                   , buffer_t      *input
                   , buffer_t      *outputs[7]
                   , spinn_coord_t  position
                   , bool           use_emg_routing
                   , int            first_timeout
                   , int            final_timeout
                   , void           (*on_forward)( spinn_router_t    *router
                                                 , spinn_packet_t    *packet
                                                 , void              *data
                                                 )
                   , void           *on_forward_data
                   , void           (*on_drop)( spinn_router_t *router
                                              , spinn_packet_t *packet
                                              , void           *data
                                              )
                   , void           *on_drop_data
                   )
{
	// Allocate the datastructure
	spinn_router_t *r = malloc(sizeof(spinn_router_t));
	assert(r != NULL);
	
	// Initialise internal fields
	r->time_elapsed              = 0;
	r->forward_packet            = false;
	r->drop_packet               = false;
	
	// Copy fields from parameters
	r->input = input;
	memcpy(r->outputs, outputs, sizeof(buffer_t *) * 7);
	
	r->position    = position;
	
	r->use_emg_routing = use_emg_routing;
	r->first_timeout   = first_timeout;
	r->final_timeout   = final_timeout;
	
	r->on_forward      = on_forward;
	r->on_forward_data = on_forward_data;
	
	r->on_drop      = on_drop;
	r->on_drop_data = on_drop_data;
	
	// Set up tick/tock callbacks in the scheduler
	scheduler_schedule( s, period
	                  , spinn_router_tick, (void *)r
	                  , spinn_router_tock, (void *)r
	                  );
	
	
	return r;
}


spinn_coord_t
router_get_position(spinn_router_t *r)
{
	return r->position;
}


void
spinn_router_free(spinn_router_t *r)
{
	free(r);
}


