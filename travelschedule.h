#ifndef TRAVELSCHEDULE_H_INCLUDED
#define TRAVELSCHEDULE_H_INCLUDED
/* A program to model agents moving between places
    Copyright (C) 2021  Mike Bithell

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    */

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @file travelschedule.h 
 * @brief File containing the definition of the travelschedule class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief A simple fixed travel schedule that rotates cyclically between places
 * 
 * The schedule is simply an ordered list of the types of place to be visited. The actual places are stored with the individual agents, \n 
 * so that each agent can have a different location corresponding to a given type of place \n
 * Each agent sets up and stores its own copy of this schedule \n
 * Every time \ref getNextLocation is called, the schedule advances one place forward. When the end is reached the schedule resets to the first entry.
 * 
    @todo make this a singleton to save memory? would this work for OMP llel? Would imply needing modification rules for individual agents...
*/
#include<vector>

class travelSchedule{
    /** A vector of named integers that holds the default travel destinations in the order that places will be visited \n
     * The names are taken from the agent class, where and enum defines the names \n
     * Note that vector indices start at 0
     */
    std::vector<agent::placeTypes> destinations; 
    /** For each place there is a time spent at each location (in hours at the moment), held here     */
    std::vector<double>timeSpent;
    /** The name of the destination currently pointed at by this travel schedule */
    agent::placeTypes currentDestination;
    /** An index into destinations and/or timeSpent vector for the current destination */
    int index;
public:
    /** @brief Constructor to build the schedule 
     *  add the placeTypes to be visited in order to the destinations vector, and the corresponding time that will be spent in each place to the timeSpent vector. \n
     *  schedule time in a given place should be an integer multiple of the timestep. Note values here are currently set in hours - sincce one should use real time\n
     *  units to map onto actual output times\n
     *  Index is set to point to the last place so that a call to getNextLocation will run the schedule back to the top.
     */
    travelSchedule(){
        destinations.push_back(agent::vehicle);//load people into busstop (or transportHub) rather than direct into bus? - here they would wait
        timeSpent.push_back(1*timeStep::hour());
        destinations.push_back(agent::work);//trip chaining? how to handle trips across multiple transport hubs? how to do schools (do parents load up childer?) and shops? - use a stack to modify default schedule!
        timeSpent.push_back(8*timeStep::hour());
        destinations.push_back(agent::vehicle);
        timeSpent.push_back(1*timeStep::hour());
        destinations.push_back(agent::home);
        timeSpent.push_back(14*timeStep::hour());
        currentDestination=agent::home;
        index=2;//start on the bus to home - call to initTravelSchedule in agent constructor will move the agent to home for the first step
    }
    /** advance the schedule to the next place and return the placeType for that place */
    agent::placeTypes getNextLocation(){
        index++;
        index=index%destinations.size();
        currentDestination=destinations[index];
        return currentDestination;
    }
    /** report the time spent at the current place */
    double getTimeAtCurrentPlace(){
        return timeSpent[index];
    }

};
#endif // TRAVELSCHEDULE_H_INCLUDED
