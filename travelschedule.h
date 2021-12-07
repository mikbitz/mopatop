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
 * @brief File containing the definition of the travelSchedule class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief A simple fixed travel schedule that rotates cyclically between places
 * 
 * The schedule is simply an ordered list of the \b types of place to be visited. The actual places are stored with the individual agents, \n 
 * so that each agent can have a different location corresponding to a given type of place \n
 * Each agent sets up and stores its own index into this schedule \n
 * Every time \ref getNextLocation is called, the schedule reports one place forward. When the end is reached the schedule resets to the first entry.\n
 * It is up to the agent to keep track of where it has got to in the schedule.\n
 * Agents can switch between a limited number of pre-defined schedules, currently hard-coded here. At the moment there is a generic list of schedules that agents can use \n
 * see schedulelist.h (saves memory over each agent having a copy of the schedule) - but agents might vary whether they move through the schedule or not.
    @todo schedule based on abolute times rather than time spent in a place would be more malleable (add events in sorted order of time, etc.) - but then agents each need an individual schedule, potentially.
*/
#include<vector>
#include "agent.h"
#include "schedulelist.h"
class travelSchedule{
    /** A vector of named integers that holds the default travel destinations in the order that places will be visited \n
     * The names are taken from the agent class, where and enum defines the names \n
     * Note that vector indices start at 0
     */
    std::vector<agent::placeTypes> destinations; 
    /** For each place there is a time spent at each location (in hours at the moment), held here     */
    std::vector<double>timeSpent;
    /** The name of the destination if no other information is given, pointed at by this travel schedule */
    agent::placeTypes defaultDestination;
    /** An index into destinations and/or timeSpent vector for the first destination at the schedule intialisation */
    int index;
public:
    /** @brief Default constructor to build the schedule 
     *  @details add the placeTypes to be visited in order to the destinations vector, and the corresponding time that will be spent in each place to the timeSpent vector. \n
     *  schedule time in a given place should be an integer multiple of the timestep. Note values here are currently set in hours - since one should use real time\n
     *  units to map onto actual output times\n
     *  Index is set to point to the last place visited so that a call to getNextLocation will report the follow on (maybe back to the start).
     *  The default schedule is set to agents remaining in one place see \ref stationary
     */
    travelSchedule(){
        stationary();
    }
    /** @brief Constructor to build schedules of a given type, to be added to the schedule list
     *  @details use the schedule types from the list to instantiate one of the possible types of schedule
     */
    travelSchedule(scheduleList::scheduleTypes i){
        if (i==scheduleList::mobile)mobile();
        if (i==scheduleList::stationary)stationary();
        if (i==scheduleList::remoteTravel)remoteTravel();
        if (i==scheduleList::returnTrip)returnTrip();
    }
    /** @brief Change the travel schedule, picking the new one by name
     *@param s The name fo the schedule to be used from now on */
    void switchTo(std::string s){
        if      (s=="mobile"      )mobile();
        else if (s=="stationary"  )stationary();
        else if (s=="remoteTravel")remoteTravel();
        else if (s=="returnTrip"  )returnTrip();
        else {
            std::cout<<"Unknown schedule name in switchTo: "<<s<<std::endl;            
        }
    }
    /** @brief remove all data for the current schedule
     *  @details This leads to an emrpty schedule - it is assumed that the detination should therefore be fixed as home \n
        Time spent there will be reported as zero */
    void cleanOldSchedule(){
        destinations.clear();
        timeSpent.clear();
        defaultDestination=agent::home;
    }
    /** @brief A schedule where the agent just stays at home the entire time */
    void stationary(){
        cleanOldSchedule();
        destinations.push_back(agent::home);
        timeSpent.push_back(24*timeStep::hour());
        defaultDestination=agent::home;
        index=0;//stay at home the entire time
    }
    /** @brief A schedule where the agents moves between work and home using their chosen vehicle */
    void mobile(){
        cleanOldSchedule();
        destinations.push_back(agent::vehicle);//load people into busstop (or transportHub) rather than direct into bus? - here they would wait
        timeSpent.push_back(1*timeStep::hour());
        destinations.push_back(agent::work);//trip chaining? how to handle trips across multiple transport hubs? how to do schools (do parents load up childer?) and shops? - use a stack to modify default schedule!
        timeSpent.push_back(8*timeStep::hour());
        destinations.push_back(agent::vehicle);
        timeSpent.push_back(1*timeStep::hour());
        destinations.push_back(agent::home);
        timeSpent.push_back(14*timeStep::hour());
        defaultDestination=agent::vehicle;//on the way home...
        index=2;//start on the bus to home - call to initTravelSchedule in agent constructor will move the agent to home for the first step
    }
    /** @brief A schedule where the agents go to an alternative destination away from home */
    void remoteTravel(){
        cleanOldSchedule();
        destinations.push_back(agent::vehicle);
        timeSpent.push_back(8*timeStep::hour());
        destinations.push_back(agent::home);
        timeSpent.push_back((16+5*24)*timeStep::hour());
        defaultDestination=agent::vehicle;
        index=1;//take a long trip, then stay at work the entire time for five days - but now work is at a hotel
    }
    /** @brief Agents return from their time away from home */
    void returnTrip(){
        cleanOldSchedule();
        destinations.push_back(agent::vehicle);
        timeSpent.push_back(8*timeStep::hour());
        destinations.push_back(agent::home);
        timeSpent.push_back(2*timeStep::hour());
        defaultDestination=agent::vehicle;
        index=1;//vehicle returns agent, then they stay at home for two hours (for compatibility with mobile schedule).
    }
    /** @brief return the placeType for the next place in the schedule after postion n. Schedule is cyclic and is meant to represent a whole day.
        @details The schedule report the next destination when this function is called. If the step goes beyond the schedule end, it wraps back to the start.
        @param n an index into the list of destinations*/
    agent::placeTypes getNextLocation(unsigned n){
        agent::placeTypes p=defaultDestination;
        if (destinations.size()>0){
         n++;
         n=n%destinations.size();
         p=destinations[n];
        }
        return p;
    }
    /** return the placeType for the current target */
    agent::placeTypes getCurrentDestination(unsigned n){
        if (destinations.size()==0) return defaultDestination;
        n=n%destinations.size();
        return destinations[n];
    }
    /** report the time spent at the current placeType */
    double getTimeAtCurrentPlace(unsigned n){
        if (destinations.size()==0) return 0;
        n=n%destinations.size();
        return timeSpent[n];
    }
    /** increment a counter that tracks the schedule location, including wrap-around */
    double increment(unsigned n){
        if (destinations.size()==0) return 0;
        n++;
        n=n%destinations.size();
        return n;
    }
    /** return the index point in the schedule from its initial state */
    unsigned getStartPoint(){
        return index;
    }
};
#endif // TRAVELSCHEDULE_H_INCLUDED
