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

class place;
#include"disease.h"
class activityType;
/**
 * @brief The main agent class - each agent represents one person
 * @details Agents move from place to place, using the travelSchedule. If they have the disease, the cough at each place they visit and contaminate it \n
 * If they are in a contaminated location, they may contract the disease. Additionally they may do other things in their current location.
*/
class agent{
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
    /** @brief create and agent and set default disease flags and ID. 
     * @details The static nextID variable is used to auto-set the ID number. nextID is then incremented.\n
     * Also set aside storage for the three placeTypes the agent can occupy. \n
     *  these are set later, as the places need to be created before they can be allocated to agents.\n
     * NB this means that places is initially empty - remember to set agent home/work/transport before anything else happens!\n
     */
    agent();
     /** @brief Set the value of \ref nextID 
         @details Use with caution - resetting this will cause automatic agent IDs to be set starting from the value set here \n
         If agent IDs need to be unique then this could cause issues... need to take care if agents travel to another MPI domain
         @param i the new value from which agent IDs will be auto-incremented.*/
    static void setIDbaseValue(unsigned long i){
        nextID=i;
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
    /** @brief Unique agent identifier - should be able to go up to 4e9 */
    unsigned long ID;
    /** @brief This enum identifies whether agents move from place to place
     * @details By default this is true - the idea is that agents can be set to stay in a single location for testing against the simplest single-lcation disease model */
    bool _mobile;
    /** @brief This enum associates a set of integers with names, in order to identify types of place. 
     * @details So home=0, work=1 etc. This allows meaningful names to be used to refer to the type of place the agent currently occupies, for example.
     * Each agent has its own mapping from the placeType to an actual place - so home for agent 0 can be a different place for home for agent 124567.
     * vehicles,transport are places, albeit moveable!
     * some of the places tha agent needs to remember permanently (e.g. home, work, vehicle), but other (e.g. travel hub) might be temporary - cache is provided for
     * some placetypes that might be needed later or don't fit in the current named list.
     * some other structure would be ideal...at the moment though, this enum is used in rules to detect where the agent is currently, so useful to have all placetypes in one enum
     * so that the dataype in if-blocks is the same for all*/
    enum placeTypes{home,work,vehicle,hospital,shop,travelhub,transport,accomodation,destination,cache};
    /** @brief An array of pointers to places 
     *  @details - indexed using the placeType, so that the integer value doesn't need to be used - instead one can use the name (home.work etc.) \n
       intially these places are null pointers, so care must be taken to initialise them in the model class, once places are available (otherwise the model will likely crash at some point!).
       Note that this could be replaced with a vector for more flexibility, but this is slightly slower and consumes more memory. 
       Note that using STL (e.g. map<string,place*>  would work also (and be better), but is much memory expensive.
        Some better way to do this kind of named indexed structure is needed though...
        Array need to match placetype enum in size*/
    place* places[10];
    /** @brief Where the agent is currently located 
     *@details - note to get this actual place, use this as an index into the places vector*/
    placeTypes currentPlace;
    /** @brief A rule to determine whether the agent is about to go away on holiday*/
    void goOnHoliday();
    /** @brief A rule to determine whether the agent is about to go get on plane home*/
    void returnFromHoliday();
    /** @brief A rule to determine whether the agent has reached home*/
    void arriveHome();
    /** @brief A rule to determine whether the agent is about to go away on travel 
        @details if true switches the ruleset to a holiday version */
    bool holidayTime();
    /** @brief set up travelling away
        @details this sets a schedule and a set of places to visit. When there are multiple MPI domains, the copy of a remote agent needs to run this\n
        with the local places so as the make sure the traveller agent gets places that exist on this domain (see \ref fetchall.h). The local agents also calls\n
        this routine, which allows it to set *local* places that will be ready for it on return, including if it comes back from a remote domain.*/
    void outwardTravel();
    /** @brief set up returning home 
       @details used by returning travellers. If they have come back from a remote MPI domain, their cached return flight can now be used (as set in outward travel)\n
       note that this is called from \ref fetchall.h aslo*/
    void inwardTravel();
    /** @brief set a flag to indicate there's a need to move to another MPI domain
     @details used by \ref fetchall.h for incoming travellers, and by the holidayTime method, where it detects from the remoteTravel object whether it is actually on another domain*/
    void setRemoteLocation();
    /** @brief variable set to true if needing to cross domains */
    bool _locationIsRemote;
    //the next functions are defined after the travelSchedule class, as they need to know the schedule details before they can be set up
    /** @brief Move through the travel schedule, and then do any actions specific to places (apart from disease) \n
        @details needs to be called every timestep see \ref agent.cpp for definition\n 
        */
    void update();
    /** @brief initialise agetn data using the parameterSettings \n
     *  @details The agents do not keep a handle to the parameters, so this allows any initial agent data that depends on parameters to be set up
        @param params A reference to a parameterSettings object  
        see \ref agent.cpp for definition*/
    void initialize(parameterSettings& );
    /** @brief check whether to move forward to the next travel location
        @details decrement the time counter for the current place by the timestep and then check to see if this time has expired\n
        If so get the next place on the schedule as pointed to by schedulePoint+1
        see \ref agent.cpp for definition*/
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
    void atHome();
    /** @brief do any things that need to be done at work */
    void atWork();
    /** @brief do any things that need to be done while travelling */
    void inTransit();
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
       return places[home];
    }
    /** @brief  get the place corresponding to work       
     *@return pointer to a place*/
    place* getWork(){
       return places[work];
    }
    /**  @brief get the place corresponding to transport vehicle      
     *@return pointer to a place*/
    place* getTransport(){
       return places[vehicle];
    }
    /**  @brief get the place corresponding to where the agent is now      
     *@return pointer to a place*/
    place* getCurrentPlace(){
       return places[currentPlace];
    }
    /** @brief return whether the agent is about to emigrate  
     @return boolean true if agent is leaving the domain */
    bool leaver(){
        return _leaver;
    }
    /** @brief set the agent to leave the MPI domain on the next timestep   */
    void leaveDomain(){
         _leaver=true;
    }
    /** @brief set the agent to remain in the MPI domain on the next timestep   */
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
    /** @brief check if and agent needs to go to hospital   
        @details placeholder for future development */
    bool verySick(){
        return false;
    }
    /** @brief if hospitalization is needed, travel to the relevant hospital
        @details placeholder for future development */
    void goToHospital(){
    }
    /** @brief perhaps we have recovered - go home
        @details placeholder for future development */
    void leaveHospital(){
    }
    /** @brief what to do in hospital
        @details placeholder for future development */
    void inHospital(){
    }
    /** @brief should we go to the shops?
        @details placeholder for future development */
    bool needFood(){
        return false;
    }
    /** @brief travel to the relevant shopping location
        @details placeholder for future development */
    void goShopping(){
    }
    /** @brief get shopping
     *  @details placeholder for future development */
    void inShop(){
    }
    /** @brief leave shopping location
        @details placeholder for future development */
    void leaveShop(){
    }
    /** @brief Function to change the agent from one place's list of occupants to another 
     *  @details- not used just at present - this function is very expensive on compute time \n
     * as it has to remove agent from one list and add it to another  see \ref agent.cpp for definition*/
    void moveTo(placeTypes);
};


#endif // AGENT_H_INCLUDED
