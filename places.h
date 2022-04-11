#ifndef PLACES_H_INCLUDED
#define PLACES_H_INCLUDED
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
/**
 * @file places.h 
 * @brief File containing the deinfition of the \ref place class
 * @details Note that some of the method implementations (that depend on the agent class) are currently in agent.cpp
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#include<set>
#include<math.h>
#include "timestep.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//Forward declaration of agent as they are needed in place class
//This is a C++ idiom where two classes refer to each other, so neither can be cleanly set up first
//This declaration allows the place class to know that agents exist, but not their structure


class agent;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief Places can have a list of occupants, and store disease contamination
 * @details At the moment places are disjoint from each other - agents travel between them on a schedule, but don't interact with other not in the same place. \n
 * At present the interaction is indirect through contamination that agents in a given place can leave there, if they are carying a disease. \n
 * Places incliude mobile entities such as buses - so the schedule for movement between places needs to include explicit getting into vehicles, and a specifcation of time spent there, \n
 * as contamination level should depend on the duration of stay in a given place.
*/
class place{

    /** @brief Unique identifier for a place - should be able to go up to about 4e9 */
    unsigned long ID;
    /** @brief an integer used to label the kind of place this is
        @details defaults to 0 as being no particular kind of location **/
    unsigned short category;
    /** @brief An arbitrary number giving how infectious a given place currently might be 
     *  @details - needs calibration to get a suitable per-unit-time value. \n 
     One might expect it to vary with the size of a given location
     */
    double contaminationLevel;
    /** Rate of decrease of contamination - per time step (exponential) \n */
    double fractionalDecrement;
    /** @brief This flag is used to clear out any contamination at the start of every timestep, if required
     * @details for example, if one wants the contamination level to be just proportional to the current number\n
     * of agents in a place at the opint where agents test for infection, set this to true.  
     */
    bool cleanEveryStep;
    /** unique list of current people in this place - intended for direct agent-agent interaction \n
     * For the current disease model this is not needed, as the agents need only know where they are to contaminate a place \n
     * currently this is not used...seems to add about 20% to memory requirement if populated. \n
     * A std::set is unique - so the same agent can be added many times but will only apear once.\n
     * The set uses the pointer to a given agent as the key, so its easy to insert or remove arbitrary agents.
     Currently commented out everywhere to save memory*/
    //std::set<agent*> occupants;
public:
    /** @brief set up the place. 
     *  @details Assumed initially clean. The decrement value might very with place type and ventialtion level...\n
        but here is set to a fixed number*/
    place(){
        ID=0;
        contaminationLevel=0.;
        fractionalDecrement=0.1;
        cleanEveryStep=false;
        category=0;
    }
    /** @brief set up the place. 
     *  @param p parameter Settings read from the parameter file 
     *  @details Assumed initially clean. The decrement value is here imported from the parameter settings */
    place(parameterSettings p){
        ID=0;
        contaminationLevel=0.;
        fractionalDecrement=p.get<double>("places.disease.simplistic.fractionalDecrement");
        cleanEveryStep=p.get<bool>("places.cleanContamination");        
    }
    /** @brief set the place ID number
     *  @details care shoudl be taken that ths value set here is unique! */
    void setID(long i){
        ID=i;
    }
    /** @brief get the place ID number */
    long getID(){
        return ID;
    }
    /** @brief set the place category
     *  @details care should be taken not to exceed the size of an unsigned short integer shold be something liek 0 to 65534
        @param andun signed short integer*/
    void setCategory(unsigned short c){
        category=c;
    }
    /** @brief get the place ID number */
    long getCategory(){
        return category;
    }
    /**Add an agent to the list currently here 
     @param a a pointer to the agent to be added */
    void add(agent* a){
        //occupants.insert(a);
    }
    /**Remove an agent from the list currently here 
     @param a a pointer to the agent to be removed */
    void remove(agent* a){
       // occupants.erase(a);
    }
    /** A function to allow agents (or any other thing that points at this place) to add contamination that spreads disease
     *  Essentially a proxy for droplets in the air or surface contamination \n
     *  Agents that are infected will increase this level while this is their currentPlace , offsetting the decrease in \ref update 
     * NB negative vlues not allowed!*/
    void increaseContamination(double amount){
        //in parallel runs, make sure there is no race condition here if different threads try to update the place.
        #pragma omp atomic update
        contaminationLevel+=amount;
        if (contaminationLevel<0) contaminationLevel=0;
    }
    /** A function to allow agents (or any other thing that points at this place) to completely clean up the contamination in a given place.
     * The level gets reset to zero
     * */
    void cleanContamination(){
        contaminationLevel=0.;
    }
    /** Get the current level of contamination here
     *@return Floating point value of current contamination level. */
    double getContaminationLevel(){
        return contaminationLevel;
    }
    /** Set the place to clean every step */
    void setCleanEveryStep(){
        cleanEveryStep=true;
    }
    /** Set the place *not* to clean every step */
    void unsetCleanEveryStep(){
        cleanEveryStep=false;
    }
    /** Report whether the place cleans every step */
    bool getCleanEveryStep(){
        return cleanEveryStep;
    }
    /** Set the rate of exponential decay of contamination */
    void setFractionalDecrement(double f){
        fractionalDecrement=f;
    }
    /** report the rate of exponential decay of contamination */
    double getFractionalDecrement(){
        return fractionalDecrement;
    }
    /** Report number of agents currentl in the occupant list */
    unsigned getNumberOfOccupants(){
        return 0;//occupants.size();
    }
    /** @brief The contamination in each place decays exponentially, or is reset to zero
     * @details. This function should be called every (uniform) time step \n
     *  This way places without any currently infected agents gradually lose their infectiveness, or else if \n
     *  \ref cleanEveryStep is set, the place has all contamination removed - useful if contamination shoudl only be present\n
     *  as long as agents are present, and amount should be directly given by the number of agents.
     *  the decrement rate is assumed to be specified *PER HOUR*
     * */
    void update(){
        if (cleanEveryStep)cleanContamination();
        else contaminationLevel*=std::exp(-fractionalDecrement*timeStep::deltaT()/timeStep::hour());
    }
    /** @brief Function to show the current status of a place - use with caution if there are many thousands of places! 
        @details defined in \ref places.cpp once agents have been defined*/
    void show(bool listAll=false);
    //Because of the forward declaration of class agent, the full definition of this function has to wait until after the agent class is completed
};
#endif // PLACES_H_INCLUDED
