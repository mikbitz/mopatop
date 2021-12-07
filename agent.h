#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED


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
 * @file agent.h 
 * @brief File containing the definition of the \ref agent class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//Forward declaration of travelSchedule class, so agents know it exists - even though the travelSchedule also needs to know about agents

class travelSchedule;
class place;
#include"disease.h"
#include "schedulelist.h"

/**
 * @brief The main agent class - each agent represents one person
 * @details Agents move from place to place, using the travelSchedule. If they have the disease, the cough at each place they visit and contaminate it \n
 * If they are in a contaminated location, they may contract the disease. Additionally they may do other things in their current location.
*/
class agent{
    /** @brief A static (class-level) variable that stores a list of all possible allSchedules
        @details the list is indexed by \ref scheduleList::scheduleTypes - a single instance minmimizes storage, as the schedules themselves are rather memory expensive */
    static scheduleList allSchedules;
    /** @brief A static (class-level) variable that stores the next ID number for a new agent - initialised to 0 in agent.cpp */
    static unsigned long nextID;
    /** @brief flag set to true if the agent has the disease */
    bool _diseased;
    /** @brief flag set to false initially, set if the agent cannot catch the disease 
      @details in the simplest case an agent that recovers from the disease is immune forever */
    bool _immune;
    /** @brief flag set to false initially, and true when the agent recovers from disease */
    bool _recovered;
    /** @brief flag set to true if the agent is alive */
    bool _alive=true;
    /** @brief flag set to true if the agent is active - some agents may pause their activity if visting other domains */
    bool _active=true;
    /** @brief flag set to true if the agent is about to leave this domain */
    bool _leaver=false;
public:
     /** @brief Set the value of \ref nextID 
         @details Use with caution - resetting this will cause automatic agent IDs to be set starting from the value set here \n
         If agent IDs need to be unique then this could cause issues...
         @param i the new value from which agent IDs will be auto-incremented.*/
    static void setIDbaseValue(unsigned long i){
        nextID=i;
    }
    /** @brief Unique agent identifier - should be able to go up to 4e9 */
    unsigned long ID;

    /** @brief This enum associates a set of integers with names. 
     * @details So home=0, work=1 etc. This allows meaningful names to be used to refer to the type of place the agent currently occupies, for example.
     * Each agent has its own mapping from the placeType to an actual place - so home for agent 0 can be a different place for home for agent 124567.
     * transport vehicles are places, albeit moveable!*/
    enum placeTypes{home,work,vehicle};
    /** @brief An array of pointers to places 
     *  @details - indexed using the placeType, so that the integer value doesn't need to be used - instead one can use the name (home.work etc.) \n
       intially these places are null pointers, so care must be taken to initialise them in the model class, once places are available (otherwise the model will likely crash at some point!).
       Note that this could be replaced with a vector for more flexibility, but this is slightly slower and consumes more memory. Array need to match placetype enum in size*/
    place* places[3];
    /** @brief a stack to store temporarily any places that need to be remebered for later use. Used when agent travels outside standart routine.\n
        @details Visiting places using a \ref remoteTravel.h object resets the places stored in places to point e.g. home and vehicle to holiday destinations \n
        the cache allows original places to be remebered and restored on return from travel. Note that using an STL stack would work, but is hugely memory expensive.*/
    place* placeCache[3];
    /** @brief Where the agent is currently located 
     *@details - note to get this actual place, use this as an index into the places vector*/
    placeTypes currentPlace;
    /** @brief an integer that picks out the current step through the travel schedule */
    unsigned schedulePoint;
    /** @brief The current type of travel schedule     */
    scheduleList::scheduleTypes scheduleType;
    /** @brief Place to hold schedule type if switching current schedule to an alternative (e.g. on holiday)    */
    scheduleList::scheduleTypes originalScheduleType;
    /** @brief Counts down the time spent at the current location     */  
    double scheduleTimer=0;
    /** @brief A rule to determine whether the agent is about to go away on travel 
        @param step the current model time step*/
    bool holidayTime(long);
    /** @brief set up the schedule for travelling away */
    void outwardTravel();
    /** @brief set up the schedule for returning home */
    void inwardTravel();
    /** set a flag to indicate there's a need to move to another domain */
    void setRemoteLocation();
    /** @brief variable set to true if needing to cross domains */
    bool _locationIsRemote;
    /** @brief create and agent and set default disease flags and ID. 
     * @details The static nextID variable is used to auto-set the ID number. nextID is then incremented.\n
     * Also set aside storage for the three placeTypes the agent can occupy. \n
     *  these are set later, as the places need to be created before they can be allocated to agents.\n
     * NB this means that places is initially empty - remember to set agent home/work/transport before anything else happens!\n
     */


    agent(){
        _diseased=false;
        _immune=false;
        _recovered=false;
        _alive=true;
        _active=true;
        _leaver=false;
        _locationIsRemote=false;
        ID=nextID;
        nextID++;
    }

    /** @brief Function to change the agent from one place's list of occupants to another 
     *  @details- not used just at present - this function is very expensive on compute time 
     see \ref agent.cpp for definition*/
    void moveTo(placeTypes);
    //the next functions are defined after the travelSchedule class, as they need to know the schedule details before they can be set up
    /** @brief Move through the travel schedule, and then do any actions specific to places (apart from disease) \n
        @details needs to be called every timestep see \ref agent.cpp for definition\n 
        @param step the current model time step*/
    void update(long);

    /** @brief Change schedules according to whether routine or (for example) going on holiday \n
    *  @details If going away the original home/work/vehicle pointers are cached, then new ones selected for \n
    *  the remote destination. The originals get replaced on returning home: otherwise the schedule gets updated for the next timestep.
        see \ref agent.cpp for definition\n
        @param step the current model time step*/
    void updateTravelSchedule(long);
    /** @brief initialise the travel schedule  - this sets up the list of places that will be visited, in order \n
    *  @details The schedule is assumed to be sitting at the end of the previous event when the model starts. \n
    *  set up the timer to be zero therefore (no time left at current event) and get the start schedulePoint out of the schedule.
        see \ref agent.cpp for definition*/
    void initTravelSchedule();
    /** @brief initialise the travel schedule  but setting the scheduleType using its named string \n
    *  @details As for the 
        @param params A string that picks out one of the available travel schedules
        see \ref agent.cpp for definition*/
    void initTravelSchedule(std::string s);
    /** @brief initialise the travel schedule  - this time using the parameterSettings \n
     *  @details The schedule name is extracted from the parameters and passed as a string argument to the overloaded travel schedule initialiser
        @param params A reference to a parameterSettings object  
        see \ref agent.cpp for definition*/
    void initTravelSchedule(parameterSettings& );
    /** @brief check whether to move forward to the next travel location
        @details decrement the time counter for the current place by the timestep and then check to see if this time has expired\n
        If so get the next place on the schedule as pointed to by schedulePoint+1
        see \ref agent.cpp for definition*/
    void advanceTravelSchedule();
    /** @brief if you have the disease, contaminate the current place  - call every timestep \n
     see \ref agent.cpp for definition*/
    void cough();
    /** @brief call the disease functions, specified for this agent \n
     see \ref agent.cpp for definition*/
    void process_disease(randomizer& );
    /** @brief report whether infected with the disease */
    bool diseased(){
        return _diseased;
    }
    /** @brief report whether recovered from the disease */
    bool recovered(){
        return _recovered;
    }
    /** @brief report whether immune to the disease */
    bool immune(){
        return _immune;
    }
    /** @brief give the agent the disease
        @details needed to set off the disease initially*/
    void becomeInfected(){
        _diseased=true;
    }
    /** @brief recover from disease*/
    void recover(){
        _diseased=false ; _immune=true;  _recovered=true;
    }
    /** @brief die - possibly from any cause...
        @details set flags relevant to disease anyway as these are needed for reporting */
    void die(){
        _diseased=false ; _immune=false; _recovered=false; _alive=false;
    }
    /** @brief check if the agent is alive */
    bool alive(){
        return _alive;
    }
    /** @brief set life level - for use with agent copying  */
    void setAlive(bool life){
        _alive=life;
    }
    /** @brief set immunity - for use with agent copying */
    void setImmune(bool immunity){
        _immune=immunity;
    }
    /** @brief set disease state - for use with agent copying */
    void setDiseased(bool diseased){
        _diseased=diseased;
    }
    /** @brief set recovery state - for use with agent copying */
    void setRecovered(bool recovery){
        _recovered=recovery;
    }
    /** @brief do any things that need to be done at home */
    void atHome(){
        //if (ID==0)std::cout<<"at Home "<<std::endl;
    }
    /** @brief do any things that need to be done at work */
    void atWork(){
        //if (ID==0)std::cout<<"at Work"<<std::endl;
    }
    /** @brief do any things that need to be done while travelling */
    void inTransit(){
        //if (ID==0)std::cout<<"travelling"<<std::endl;
    }
    /** @brief set up the place vector to include being at home 
     * @details - needs to be called when places are being created by the model class 
     @param pu a pointer to the specific home location for this agent */
    void setHome(place* pu){
        places[home]=pu;
        //start all agents at home - if using the occupants list, add to the home place
        //pu->add(this);
        currentPlace=home;
    }
    /** @brief set up the place vector to include being at work 
     * @details - needs to be called when places are being created by the model class 
       @param pu a pointer to the specific work location for this agent */
    void setWork(place* pu){
        places[work]=pu;
    }
    /** @brief set up the place vector to include travelling 
     * @details - needs to be called when places are being created by the model class 
     @param pu a pointer to the specific transport (e.g. a bus) location for this agent */
    void setTransport(place* pu){
        places[vehicle]=pu;
    }
    /** @brief get the place corresponding to home
         @return pointer to a place*/
    place* getHome(){
       return places[home];//places[home];
    }
    /** @brief  get the place corresponding to work       
     *@return pointer to a place*/
    place* getWork(){
       return places[work];//places[work];
    }
    /**  @brief get the place corresponding to transport vehicle      
     *@return pointer to a place*/
    place* getTransport(){
       return places[vehicle];//places[vehicle];
    }
    /**  @brief get the place corresponding to where the agent is now      
     *@return pointer to a place*/
    place* getCurrentPlace(){
       return places[currentPlace];//places[currentPlace];
    }
    /** @brief set agent ID number  
     @param i a long integer */
    void setID(long i){
        ID=i;
    }
    /** @brief get agent ID number  
     @return The agent ID number, an unsigned long integer */
    unsigned long getID(){
        return ID;
    }
    /** @brief return whether the agent is about to emigrate  
     @return boolean true if agent is leaving the domain */
    bool leaver(){
        return _leaver;
    }
    /** @brief set the agent to leave the domain on the next timestep   */
    void leaveDomain(){
         _leaver=true;
    }
    /** @brief set the agent to remain in the domain on the next timestep   */
    void doNotLeaveDomain(){
         _leaver=false;
    }
    /** @brief agent is present, but will not do anything or engage with other agents   
        @details This is primarily for use with MPI based remote travel where agents have to move to another domain.\n
        To save re-allocating memory, agents can be present both on a local and remote MPI domain - while active \n
        on the remote domain, they are inactive locally.*/
    void deactivate(){
         _active=false;
    }
    /** @brief agent will resume activity           
     * @details This is primarily for use with MPI based remote travel where agents have to move to another domain.\n
        Agents will call this on returning to the local domain (any remote copies having been deactivated symmetrically */
    void activate(){
         _active=true;
    }
    /** @brief check if agent is active   
        @details this is checked in \ref model.h to see whether agent should be doing anything (i.e. it is currently on a local MPI domain) */
    bool active(){
         return _active;
    }
};
#endif // AGENT_H_INCLUDED
