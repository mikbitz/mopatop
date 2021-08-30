#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED
#include"places.h"
#include"disease.h"
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
 * @brief File containing the definition of the agent class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//Forward declaration of travelSchedule class, so agents know it exists - even though the travelSchedule also needs to know about agents

class travelSchedule;
/**
 * @brief The main agent class - each agent represents one person
 * @details Agents move from place to place, using the travelSchedule. If they have the disease, the cough at each place they visit and contaminate it \n
 * If they are in a contaminated location, they may contract the disease. Additionally they may do other things in their current location.
*/
class agent{
public:
    /** Unique agent identifier - should be able to go up to 4e9 */
    unsigned long ID;
    /** This enum associates a set of integers with names. So home=0, work=1 etc. This allows meaningful names to be used to refer to the type of place the agent currently occupies, for example.
     * Each agent has its own mapping from the placeType to an actual place - so home for agent 0 can be a different place for home for agent 124567.
     * transport vehicles are places, albeit moveable!*/
    enum placeTypes{home,work,vehicle};
    /** A vector of pointers to places - indexed using the placeType, so that the integer value doesn't need to be used - instead one can use he name (home.work etc.) \n
       intially these places are null pointers, so care must be taken to initialise them in the model class, once places are available (otherwise the model will likely crash at some point!)*/
    std::vector<place*>places;
    /** Where the agent is currently located - note to get this actual place, use this is as an index into the places vector*/
    placeTypes currentPlace;

    /** the default travel schedule - currently every agent has the same - 
     * @todo needs modification...(singleton?) */
    travelSchedule* schedule;
    /** Counts down the time spent at the current location
     */    
    double counter=0;
    /** flag set to true if the agent is alive */
    bool alive=true;
    /** flag set to true if the agent has the disease */
    bool diseased;
    /** flag set to false initially, and true when the agent recovers from disease */
    bool immune;
    /** create and agent and set default disease flags. Also set aside storage for the three placeTypes the agent can occupy. \n
     *  these are set later, as the places need to be created before they can be allocated to agents \n
     */
    agent(){
        diseased=false;
        immune=false;
        alive=true;
        //this has to be the same size as the placeTypes enum
        places.resize(3);
    }
    /** Function to change the agent from one place's list of occupants to another - not used just at present - this function is very expensive on compute time */
    void moveTo(placeTypes location){
        places[currentPlace]->remove(this);
        places[location]->add(this);
        currentPlace=location;
    }
    //the next three functions are defined after the travelSchedule, as they need to know the scheduel details before they can be set up
    /** Move through the travel schedule, and then do any actions specific to places (apart from disease) \n
     needs to be called every timestep */
    void update();
    /** @brief initialise the travel schedule  - this sets up the list of places that will be visited, in order 
        @param params A reference to a parameterSettings object  */
    void initTravelSchedule(parameterSettings& );
    /** if you have the disease, contaminate the current place  - call every timestep */
    void cough();
    /** call the disease functions, specified for this agent */
    void process_disease(randomizer& r){
        //recovery
        if (diseased){
            if (disease::die(r))              {diseased=false ; immune=false; alive=false;}
            if (alive && disease::recover(r)) {diseased=false ; immune=true;}
        }
        //infection
        if (alive && !immune && disease::infect(places[currentPlace]->getContaminationLevel(),r) )diseased=true;
        //immunity loss could go here...
    }
    /** do any things that need to be done at home */
    void atHome(){
        if (ID==0)std::cout<<"at Home "<<std::endl;
        
    }
    /** do any things that need to be done at work */
    void atWork(){
        if (ID==0)std::cout<<"at Work"<<std::endl;
    }
    /** do any things that need to be done while travelling */
    void inTransit(){
        if (ID==0)std::cout<<"on Bus"<<std::endl;
    }
    /** set up the place vector to include being at home - needs to be called when places are being created by the model class 
     @param pu a pointer to the specific home location for this agent */
    void setHome(place* pu){
        places[home]=pu;
        //start all agents at home - if using the occupants list, add to the home place
        //pu->add(this);
        currentPlace=home;
    }
    /** set up the place vector to include being at work - needs to be called when places are being created by the model class 
       @param pu a pointer to the specific work location for this agent */
    void setWork(place* pu){
        places[work]=pu;
    }
    /** set up the place vector to include travelling - needs to be called when places are being created by the model class 
     @param pu a pointer to the specific transport (e.g. a bus) location for this agent */
    void setTransport(place* pu){
        places[vehicle]=pu;
    }

};
#endif // AGENT_H_INCLUDED
