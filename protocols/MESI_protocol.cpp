#include "MESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MESI_protocol::MESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{this->state = MESI_CACHE_I;											// Initialize lines to not have the data yet!
}

MESI_protocol::~MESI_protocol ()
{    
}

void MESI_protocol::dump (void)
{
    const char *block_states[8] = {"X","I","S","E","M","IS","IM","SM"};
    fprintf (stderr, "MESI_protocol - state: %s\n", block_states[state]);
}

void MESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
	case MESI_CACHE_I:  do_cache_I (request); break;
    case MESI_CACHE_IM: do_cache_IM (request); break;
    case MESI_CACHE_M:  do_cache_M (request); break;
	case MESI_CACHE_S:  do_cache_S (request); break;
    case MESI_CACHE_IS: do_cache_IS (request); break;
    case MESI_CACHE_SM: do_cache_SM (request); break;
	case MESI_CACHE_E: do_cache_E (request); break;

    default:
        fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

void MESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
	case MESI_CACHE_I:  do_snoop_I (request); break;
    case MESI_CACHE_IM: do_snoop_IM (request); break;
    case MESI_CACHE_M:  do_snoop_M (request); break;
	case MESI_CACHE_S:  do_snoop_S (request); break;
    case MESI_CACHE_IS: do_snoop_IS (request); break;
    case MESI_CACHE_SM:  do_snoop_SM (request); break;
	case MESI_CACHE_E:  do_snoop_E (request); break;

    default:
    	fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

inline void MESI_protocol::do_cache_I (Mreq *request)
{
switch (request->msg) {
    case LOAD:
		send_GETS(request->addr);
		state = MESI_CACHE_IS;
		Sim->cache_misses++;											//ACTUAL CACHE MISSES
    	break;
    case STORE:
    	send_GETM(request->addr);
    	state = MESI_CACHE_IM;
    	Sim->cache_misses++;											//ACTUAL CACHE MISSES
    	break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_S (Mreq *request)
{switch (request->msg) {
    case LOAD:
		send_DATA_to_proc(request->addr);
		break;
    case STORE:
    	send_GETM(request->addr);
    	state = MESI_CACHE_SM;
    	Sim->cache_misses++;											// USED TO SHOW COHERENCE MISSES
    	break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }

}

inline void MESI_protocol::do_cache_E (Mreq *request)
{switch (request->msg) {
		case LOAD:
		send_DATA_to_proc(request->addr);
		break;
		case STORE:
		send_DATA_to_proc(request->addr);
    	
    	state = MESI_CACHE_M;
    	Sim->silent_upgrades++;												//UPGRADE TO MODIFIED STATE WITHOUT BROADCASTING ANYTHING ON BUS
    	break;
		default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
		}
}

inline void MESI_protocol::do_cache_M (Mreq *request)
{
switch (request->msg){
	case LOAD:
		
	case STORE:
		send_DATA_to_proc(request->addr);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: M state shouldn't see this message\n");
    }
}
inline void MESI_protocol::do_cache_IS (Mreq *request){
    switch (request->msg){
	case LOAD:
	case STORE:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error("Should only have one outstanding request per processor!");
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IS state shouldn't see this message\n");
    }
}
inline void MESI_protocol::do_cache_IM (Mreq *request){
    switch (request->msg) {
	case LOAD:
	case STORE:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error("Should only have one outstanding request per processor!");
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_SM (Mreq *request){
    switch (request->msg) {
	case LOAD:
	case STORE:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error("Should only have one outstanding request per processor!");
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
    }
}


inline void MESI_protocol::do_snoop_I (Mreq *request)
{
switch (request->msg){
	case GETS:
	case GETM:
	case DATA:
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_S (Mreq *request)
{
switch (request->msg){
	case GETS: 
		set_shared_line();												// INDICATES DATA IS SHARED
		break;
	case GETM:
		set_shared_line();												// INDICATES DATA IS SHARED
		state = MESI_CACHE_I;											//INVALIDATES DATA BECAUSE OF GETM
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }

}

inline void MESI_protocol::do_snoop_E (Mreq *request)
{
switch (request->msg){
case GETS: 
		state = MESI_CACHE_S;												// INDICATES DATA IS SHARED
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY DATA AND CHANGE STATE TO SHARED
		set_shared_line();
		break;
	case GETM:
		state = MESI_CACHE_I;
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY DATA AND INVALIDATE SELF
		set_shared_line();												// INDICATES DATA IS SHARED
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_M (Mreq *request)
{
switch (request->msg){
	case GETS:
		
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY THE DATA 
		state = MESI_CACHE_S;
		set_shared_line();
		break;
	case GETM:
		
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY THE DATA AND INVALIDATE SELF
		state = MESI_CACHE_I;
		set_shared_line();
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
	default: request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: M state shouldn't see this message\n");
	}
}
inline void MESI_protocol::do_snoop_IS (Mreq *request){
	switch (request->msg){
	case GETS:
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);
		if(get_shared_line())											//IF DATA IS SHARED NEXT STATE IS SHARED STATE ELSE EXCLUSIVE STATE
		state = MESI_CACHE_S;
		else
		state = MESI_CACHE_E;
	
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IS state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_snoop_IM (Mreq *request){
	switch (request->msg) {
	case GETS:
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);								//JUST SEND REQUIRED DATA TO PROCESSOR
		state = MESI_CACHE_M;
	
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_snoop_SM (Mreq *request){
	switch (request->msg){
	case GETS:set_shared_line();										//INDICATION OF DATA SHARING
				break;
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);								//JUST SEND REQUIRED DATA TO PROCESSOR
		state = MESI_CACHE_M;
	
	break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}
