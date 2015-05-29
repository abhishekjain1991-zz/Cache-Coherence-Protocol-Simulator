#include "MOESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESI_protocol::MOESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{																			// Initialize lines to not have the data yet!
this->state = MOESI_CACHE_I;
}

MOESI_protocol::~MOESI_protocol ()
{    
}

void MOESI_protocol::dump (void)
{
    const char *block_states[10] = {"X","I","S","E","O","M","IM","SM","IS","OM"};
    fprintf (stderr, "MOESI_protocol - state: %s\n", block_states[state]);
}

void MOESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
	case MOESI_CACHE_I:  do_cache_I (request); break;
	case MOESI_CACHE_S:  do_cache_S (request); break;
	case MOESI_CACHE_E:  do_cache_E (request); break;
	case MOESI_CACHE_O: do_cache_O (request); break;
	case MOESI_CACHE_M:  do_cache_M (request); break;
    case MOESI_CACHE_IS: do_cache_IS (request); break;
	case MOESI_CACHE_IM: do_cache_IM (request); break;
    case MOESI_CACHE_SM: do_cache_SM (request); break;
	case MOESI_CACHE_OM: do_cache_OM (request); break;
    default:
        fatal_error ("Invalid Cache State for MOESI Protocol\n");
    }
}

void MOESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
	case MOESI_CACHE_I:  do_snoop_I (request); break;
	case MOESI_CACHE_S:  do_snoop_S (request); break;
	case MOESI_CACHE_E:  do_snoop_E (request); break;
	case MOESI_CACHE_O:  do_snoop_O (request); break;
	case MOESI_CACHE_M:  do_snoop_M (request); break;
    case MOESI_CACHE_IS: do_snoop_IS (request); break;
	case MOESI_CACHE_IM: do_snoop_IM (request); break;
    case MOESI_CACHE_SM:  do_snoop_SM (request); break;
	case MOESI_CACHE_OM:  do_snoop_OM (request); break;
    default:
    	fatal_error ("Invalid Cache State for MOESI Protocol\n");
    }
}

inline void MOESI_protocol::do_cache_I (Mreq *request)
{
switch (request->msg) {
    case LOAD:
		send_GETS(request->addr);
		state = MOESI_CACHE_IS;
		Sim->cache_misses++;											//ACTUAL CACHE MISSES
    	break;
    case STORE:
    	send_GETM(request->addr);
    	state = MOESI_CACHE_IM;
    	Sim->cache_misses++;											//ACTUAL CACHE MISSES
    	break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_cache_S (Mreq *request)
{
switch (request->msg) {
    case LOAD:
		send_DATA_to_proc(request->addr);
		break;
    case STORE:
    	send_GETM(request->addr);
    	state = MOESI_CACHE_SM;
    	Sim->cache_misses++;											// USED TO SHOW COHERENCE MISSES
    	break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_cache_E (Mreq *request)
{
switch (request->msg) {
		case LOAD:
		send_DATA_to_proc(request->addr);
		break;
		case STORE:
		send_DATA_to_proc(request->addr);
    	state = MOESI_CACHE_M;
    	Sim->silent_upgrades++;												//UPGRADE TO MODIFIED STATE WITHOUT BROADCASTING ANYTHING ON BUS
    	break;
		default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
		}
}

inline void MOESI_protocol::do_cache_O (Mreq *request)
{
switch (request->msg) {
		case LOAD:
		send_DATA_to_proc(request->addr);
		break;
		case STORE:
		
    	send_GETM(request->addr);
    	state = MOESI_CACHE_OM;												//GO TO INTERMEDIATE STATE OM
		Sim->cache_misses++;												// USED TO SHOW COHERENCE MISSES
    	
    	break;
		default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
		}
}

inline void MOESI_protocol::do_cache_M (Mreq *request)
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
inline void MOESI_protocol::do_cache_IS (Mreq *request){
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
inline void MOESI_protocol::do_cache_IM (Mreq *request){
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

inline void MOESI_protocol::do_cache_SM (Mreq *request){
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
inline void MOESI_protocol::do_cache_OM (Mreq *request){
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

inline void MOESI_protocol::do_snoop_I (Mreq *request)
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

inline void MOESI_protocol::do_snoop_S (Mreq *request)
{
switch (request->msg){
	case GETS: 
		set_shared_line();												// INDICATES DATA IS SHARED
		break;
	case GETM:
		set_shared_line();												//INDICATES DATA IS SHARED
		state = MOESI_CACHE_I;											//INVALIDATES DATA BECAUSE OF GETM
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_E (Mreq *request)
{
switch (request->msg){
case GETS: 
		state = MOESI_CACHE_S;
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY DATA AND CHANGE STATE TO SHARED
		set_shared_line();
		break;
	case GETM:
		
		state = MOESI_CACHE_I;
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY DATA AND INVALIDATE SELF
		set_shared_line();
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_O (Mreq *request)
{
switch (request->msg){
	case GETS:
		set_shared_line();
		send_DATA_on_bus(request->addr,request->src_mid);				// BEING THE OWNER I SUPPLY THE DATA
		
		break;
	case GETM:
		set_shared_line();
		send_DATA_on_bus(request->addr,request->src_mid);				// BEING THE OWNER I SUPPLY THE DATA AND INVALIDATE SELF
		state = MOESI_CACHE_I;
		
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: O state shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_snoop_M (Mreq *request)
{
switch (request->msg){
	case GETS:
		
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY THE DATA SINCE I HAVE THE UPDATED VERSION
		state = MOESI_CACHE_O;											//I SUPPLY THE DIRTY DATA TO EVERYONE
		set_shared_line();
		break;
	case GETM:
		
		send_DATA_on_bus(request->addr,request->src_mid);				//SUPPLY DATA
		state = MOESI_CACHE_I;											//INVALIDATE
		set_shared_line();
		break;
	case DATA:
		fatal_error ("Should not see data for this line!  I have the line!");
		break;
	default: request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: M state shouldn't see this message\n");
	}

}

inline void MOESI_protocol::do_snoop_IS (Mreq *request){
	switch (request->msg){
	case GETS:
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);
		if(get_shared_line())											//IF DATA IS SHARED NEXT STATE IS SHARED STATE ELSE EXCLUSIVE STATE
		state = MOESI_CACHE_S;
		else
		state = MOESI_CACHE_E;
	
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IS state shouldn't see this message\n");
	}
}

inline void MOESI_protocol::do_snoop_IM (Mreq *request){
	switch (request->msg) {
	case GETS:
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);								//JUST SEND REQUIRED DATA TO PROCESSOR
		state = MOESI_CACHE_M;

		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}
inline void MOESI_protocol::do_snoop_SM (Mreq *request){
	switch (request->msg){
	case GETS:set_shared_line();										//INDICATION OF DATA SHARING
				break;
	case GETM:
		break;
	case DATA:
		send_DATA_to_proc(request->addr);								//JUST SEND REQUIRED DATA TO PROCESSOR
		state = MOESI_CACHE_M;
	
	break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void MOESI_protocol::do_snoop_OM (Mreq *request){
	switch (request->msg){
	case GETS:send_DATA_on_bus(request->addr,request->src_mid);			//SEND DATA ON BUS SINCE I'M IN INTERMEDIATE STATE BETWEEN O AND M
				break;
	case GETM:send_DATA_on_bus(request->addr,request->src_mid);			//SEND DATA ON BUS SINCE I'M IN INTERMEDIATE STATE BETWEEN O AND M AND INVALIDATE SELF
			  state = MOESI_CACHE_IM;
		break;
	case DATA:
	
	break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}



