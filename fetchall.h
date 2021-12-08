/*****************************************************************************
* Multiscale Universal Interface Code Coupling Library Demo 8                *
*                                                                            *
* Copyright (C) 2020 S. M. Longshaw                                          *
*                                                                            *
* This software is jointly licensed under the Apache License, Version 2.0    *
* and the GNU General Public License version 3, you may use it according     *
* to either.                                                                 *
*                                                                            *
* ** Apache License, version 2.0 **                                          *
*                                                                            *
* Licensed under the Apache License, Version 2.0 (the "License");            *
* you may not use this file except in compliance with the License.           *
* You may obtain a copy of the License at                                    *
*                                                                            *
* http://www.apache.org/licenses/LICENSE-2.0                                 *
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
*                                                                            *
* ** GNU General Public License, version 3 **                                *
*                                                                            *
* This program is free software: you can redistribute it and/or modify       *
* it under the terms of the GNU General Public License as published by       *
* the Free Software Foundation, either version 3 of the License, or          *
* (at your option) any later version.                                        *
*                                                                            *
* This program is distributed in the hope that it will be useful,            *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
* GNU General Public License for more details.                               *
*                                                                            *
* You should have received a copy of the GNU General Public License          *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
******************************************************************************/

/**
 * @file fetchall.h
 * @author S. M. Longshaw, Mike Bithell
 * @date 25 September 2020,25/11/2021
 * @brief originally a simple example demonstrating the non-interpolating
 *        fetch_points and fetch_values commands in MUI that are useful
 *        when just wanting to use MUI to pass data without using spatial
 *        interpolation.\n
 * @details MB 25/11/2021 updated the fetch for tranferring some agent data\n
 * Each data value transferred has an identifying point ID associated with it\n
 * Example run, with domains a and b\n
 * mpiexec -f hosts -np 1 ./testagent parameterFilea a : -np 1 ./testagent parameterFileb b
 *
 */

#include "../mui/mui.h"
#include "mui_config.h"
#include <omp.h>
/** @brief This class defines an interface for tow copes of the model running on different MPI threads
    @details On each MPI thread (currently just two allowed) define an interface for exchange of data between the threads \n
             In each time step, the list of agents is checked to see whether data should be transferred between threads*/
class MUIcoupler {
    /** @brief the interface for exchanging data, using the configuration in config.h */
    mui::uniface<mui::mui_config>* interface;
    /** @brief the name of this domain - unique to each mpi thread */
    std::string domain;
    /** @brief set to true to print out (lots of) diagnostic info - only really for debug/test purposes */
    bool verbose;
public:
    /** @brief default constructor - not used in practice */
    MUIcoupler(){verbose=false;};
    /** @brief constructor - sets up the interface connecting the two MPI domains (threads)
     * @param ident a string used to define the domain
     * @param verby set to true for verbose output - defaults to false if not presentfrom caller */
    MUIcoupler(std::string ident,bool verby=false):domain(ident),verbose(verby){
        //ident is the subdomain of mpi - needs to be unique for every instance of the code
        std::string iface="mpi://cough"+ident+"/iface";
        //create the interface for transfer of data between 2 domains
        interface=new mui::uniface<mui::mui_config> ( iface.c_str() );
    }
//--------------------------------------------------------------------------------------------------------
    /** @brief The data to be pushed from this domain to the remote domain on the other thread, for each agent */
    void push_data(int travelType, mui::point<mui::mui_config::REAL, 1>loc, agent* a){
        //send whether agent is local or traveller
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(travelType)  );
        //send agent ID
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(a->getID())  );
        //...send other necessary data here...make sure it matches below in pull_data
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(a->alive())  );
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(a->diseased())  );
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(a->immune())  );
        interface->push( "data", loc, static_cast<mui::mui_config::REAL>(a->recovered())  ); 
    }
//--------------------------------------------------------------------------------------------------------
    /** @brief The data to be received on this domain from the remote domain on the other thread, for each agent, but excluding agent ID and traveller indicator
        @details otherwise the lines here should match those in the pull_data function */
    void pull_data(unsigned long & i, agent* a,std::vector<double>& fetch_vals){
        //use the data fetched from the remote domain to set values in the agents here
        //one i++ and one line here to correspond to each in push_data above  (NB travelType and ID already set below)      
        i++;
        a->setAlive((bool)fetch_vals[i]);
        i++;
        a->setDiseased((bool)fetch_vals[i]);
        i++;
        a->setImmune((bool)fetch_vals[i]);
        i++;
        a->setRecovered((bool)fetch_vals[i]);
    }
//--------------------------------------------------------------------------------------------------------
    /** @brief check through all agents looking for exchanges and then transferring a subset of data as defined by \ref push_data and \ref pull_data 
     *  @details this gets annoyingly complex :(. When an agent leaves its original domain, it is assumed it will return at some point\n
     *  i.e. each moving agent is a traveller. For this reason the original agent on the original domain is left untouched, but labelled as inactive \n
     *  Any agents (either local ones, or travellers) that are set to leave their current domain in this timestep are initially labelled as leavers.\n 
        In the first part of the routine, all agents that are inactive have their ID stored. This is so that incoming data can be assigned to \n
        returning agents when they come back from travel. Then local leaver agents have their data pushed out to the remote domain, labelled \n
        with a zero to show that they are outgoing, and with their *local* agent ID (note this may coincide with a remote ID, but such remote agents\n
        will not be "travellers"). Following this travellers on the current domain that are labelled as leaving (i.e. they are about to go home) \n
        have their data pushed, now with a label larger than 1 to show that they are not local to this pushing thread (larger than one is used since the\n
        labels are currently doubles, so guarantees that we can distinguish this value reliably from 0). These leaving agents are immediately deactivated\n
        on the local thread (since they are about to go away to the remote MPI domain), and also re-labelled as non-leavers (since they cannot leave their\n
        domain a second time, unless and until they return to the current domain).\n
        All changes are now committed - i.e. they are sent to the remote domain - this implies a wait for the other thread to be ready.\n
        In the second part of the routine, data can now be fetched - this is all done in one go, and the data stored ready for use: it needs to be allocated\n
        to the right agents. First we deal with incoming travellers. On the remote thread, these were labelled with zero (since there they were local leavers)\n
        so we pick out those arrivals first. We need to create a new agent on the local thread to store their data - this agent is added to the travellers present here\n
        However, to save memory allocations, we can re-use any local inactive travellers - these were agents that were sent here, but have since gone home.\n
        So first we set up a list of inactive local travellers and re-use these - if we run out of these, then we create some new agents. Each traveller is \n
        added to the local vector of travellers (kept separate from the local list of agents) and assigned the ID from the remote domain. Then their schedules\n
        are set up using a call to \ref outwardTravel in \ref agent.cpp . They are also labelled as needing to leave the domain at the end of their schedule.\n
        Now we can add the returning travellers - we use their (local) ID and index, as transmitted with them from the remote domain to find their corresponding \n
        local copy, activate it and copy in the data sent from the remote domain. The schedule for returning home is set with a call to \ref inwardTravel .
        @param time The current model time step
        @param locals The list of agents local to this domain (i.e. excluding travellers)
        @param travellers The list of agents that have come from the remote domain
        @param leavers The list of new agents about to leave this domain */
    void exchange(int time,std::vector<agent*>& locals,std::vector<agent*>& travellers,std::vector<long>& leavers){
        
        // Declare MUI interface and samplers using templates in config.h
        // note: please update types stored in default_config in config.h first to 1-dimensional before compilation
        
        //no time interpolation - just exactly copy values at each time step
        mui::chrono_sampler_exact<mui::mui_config> chrono_sampler;
        mui::point<mui::mui_config::REAL, 1> push_loc;
        
        
        // Push values to the MUI interface - at the moment it seems all values have to be cast to doubles??
        
        // For each value sent, the receiver will get (in order) a copy of the location and then the value
//TODO set up the airplane (or other vehicle) to carry the agent at each end, in a way that can generalize to more than two domains, and that knows the domain.
        //count of agents moved this time
        unsigned count=0;
        // store a mapping from agent IDs to index in locals */
        std::map<unsigned long,unsigned long> identities;
        //this *might* be more efficient in parallel - as numbers returning should be a lot smaller than locals.size()
        #pragma omp parallel for
        for(unsigned long i=0; i<locals.size(); i++) { //
            //need to store the index value currently associated with inactive agents for retrieving travellers below
            //note this might not be the index they originally had when they first left the domain...
            #pragma omp critical
            if (!locals[i]->active())identities[locals[i]->getID()]=i;
        }
        //loop over all locals  leavers- Agents that normally reside on this domain may decide to leave
        //leavers vector holds the indices into the locals vector
        for(unsigned long i=0; i<leavers.size(); i++){
            //push leaver data
            count++;
            //local copy on this domain pauses
            locals[leavers[i]]->deactivate();
            //once the agent has crossed to the other side, it shouldn't try again! - until its schedule says so in the main model
            locals[leavers[i]]->doNotLeaveDomain();
            //currently use i to label the agent - OK since here we loop over all agents, and immediately fetch below
            push_loc = static_cast<mui::mui_config::REAL>(leavers[i]);
            //locals going travelling are labelled with a zero
            push_data(0,push_loc,locals[leavers[i]]);            
        }
        //Now check if travellers are on the way home from this domain
        //we keep the vector of travellers so that they can be re-used for other agents rather than re-allocating memory (which tends to be slow).
        //once in a while it might be good to clean things out??
        for(unsigned long i=0; i<travellers.size(); i++) { //temporary travellers may go home
            if (travellers[i]->leaver()){
                count++;
                //local copy on this domain pauses - this traveller can be re-used if necessary
                travellers[i]->deactivate();
                //once the agent has crossed to the other side, it shouldn't try again! - unless it gets re-used to represent another agent from the remote domain
                travellers[i]->doNotLeaveDomain();
                //dummy location - not used at the far end as the agent is crossing back to its own original domain
                push_loc = static_cast<mui::mui_config::REAL>(i);
                //returning travellers are labelled with a 2
                push_data(2,push_loc,travellers[i]); 
            }
        }
        if(verbose)std::cout<<"Domain "<<domain<<": counted "<<count<<" leavers at step "<<time<<std::endl;

        // Commit (transmit by MPI) the values at time
        interface->commit( time );
        
        //===================================================================================
        //Fetch the values from the interface using the fetch_points and fetch_values methods
        //====================================================================================
        // (blocking until data at "t=time" exists according to chrono_sampler)
        std::vector<mui::point<mui::mui_config::REAL, 1>> fetch_locs = interface->fetch_points<mui::mui_config::REAL>( "data", time, chrono_sampler ); // Extract the locations stored in the interface at time
        std::vector<double> fetch_vals = interface->fetch_values<mui::mui_config::REAL>( "data", time, chrono_sampler ); // Extract the values stored in the interface at time

        if(verbose)std::cout<<"Domain:"<< domain<<" Total number of data elements fetched "<<fetch_locs.size()<<std::endl;
        // All values for all agents, both returning locals and new travellers are all packed together, new travellers first (labelled with 0 as the first data element)

        //These are the travellers on this domain that can be re-used, since they have been inactivated
        std::vector<unsigned long>indx;
        for (unsigned long i=0;i<travellers.size();i++){
            if( !travellers[i]->active())indx.push_back(i);
        }
        
        //count of agents moved this time
        count=0;
        //index into 
        unsigned long reuser=0;
        //iterator over ALL data elements transferred
        unsigned long i=0;
        while (i<fetch_locs.size()) {
            //locals from the other domain that are travellers to here - this initializes the travellers.
            if (fetch_vals[i]<1){
                count++;
                i++;
                agent* a;
                if (reuser<indx.size()){
                    a=travellers[indx[reuser]];//use the free indices into travellers to find agents that can be overwritten with new data
                    reuser++;
                }else{
                    a=new agent();
                    travellers.push_back(a);//only needed if no free inactive travellers 
                }
                if(verbose)printf( "domain %s fetched value %lf at location %lf time %d \n", domain.c_str(), fetch_vals[i], fetch_locs[i][0],time );
                if(verbose)std::cout<<"I am a passenger, and I ride and I ride"<<std::endl;
                a->setID(fetch_vals[i]);
                a->activate();
                a->outwardTravel();//sets the local place pointers and schedule.
                a->setRemoteLocation();//agent will leave domain at end of travel schedule
                //...copy in necessary data...one value for each after the ID
                pull_data(i,a,fetch_vals);
            }else{
                //travellers from the other domain that are coming home
                count++;
                i++;
                if(verbose)printf( "domain %s fetched value %lf at location %lf time %d \n", domain.c_str(), fetch_vals[i], fetch_locs[i][0],time );
                if(verbose)std::cout<<"You're going, you're going home"<<std::endl;
                //identities labelled the agent's place in the local domain agent vector - check this is true! NB IDs are only valid in original domain
                assert(locals[identities[fetch_vals[i]]]->getID()==(long)fetch_vals[i]);
                agent* a=locals[identities[fetch_vals[i]]];
                a->activate();
                a->inwardTravel();//sets up return journey
                //...copy in modified data...one value for each after the ID
                pull_data(i,a,fetch_vals);
            }
            i++;
        }
        if(verbose)std::cout<<"Domain "<<domain<<": counted "<<count<<" arrivals at step "<<time<<std::endl;

    }
};
