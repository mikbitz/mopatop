/* A program to model agents moveing between places
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
 * @file main.cpp 
 * @brief File containing all classes and the main function
 * 
 * @author Mike Bithell
 * @date 10/08/2021
 **/
#include<iostream>
#include<algorithm>
#include<map>
#include<fstream>
#include<vector>
#include<set>
#include<string>
#include<assert.h>
#include<omp.h>
#include<filesystem>
#include"parameters.h"
#include"timereporter.h"
#include"randomizer.h"
#include"disease.h"

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

    /** Unique identifier for a place - should be able to go up to about 4e9 */
    unsigned long ID;
    /** An arbitrary number giving how infectious a given place currently might be - needs calibration to get a suitable per-unit-time value. \n 
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
     * The set uses the pointer to a given agent as the key, so its easy to insert or remove arbitrary agents */
    std::set<agent*> occupants;
public:
    /** @brief set up the place. 
     *  @details Assumed initially clean. The decrement value might very with place type and ventialtion level...\n
        but here is set to a fixed number*/
    place(){
        ID=0;
        contaminationLevel=0.;
        fractionalDecrement=0.1;
        cleanEveryStep=false;
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
    /**Add an agent to the list currently here 
     @param a a pointer to the agent to be added */
    void add(agent* a){
        occupants.insert(a);
    }
    /**Remove an agent from the list currently here 
     @param a a pointer to the agent to be removed */
    void remove(agent* a){
        occupants.erase(a);
    }
    /** A function to allow agents (or any other thing that points at this place) to add contamination that spreads disease
     *  Essentially a proxy for droplets in the air or surface contamination \n
     *  Agents that are infected will increase this level while this is their currentPlace , offsetting the decrease in \ref update */
    void increaseContamination(double amount){
        contaminationLevel+=amount;
    }
    /** A function to allow agents (or any other thing that points at this place) to completely clean up the contamination in a given place.
     * The level gets reset to zero
     * */
    void cleanContamination(){
        contaminationLevel=0.;
    }
    /** Get the current level of contamination here
     *@return Floating point value of current contamination level. */
    float getContaminationLevel(){
        return contaminationLevel;
    }
    /** @brief The contamination in each place decays exponentially, or is reset to zero
     * @details. This function should be called every (uniform) time step \n
     *  This way places without any currently infected agents gradually lose their infectiveness, or else if \n
     *  \ref cleanEveryStep is set, the place has all contamination removed - useful if contamination shoudl only be present\n
     *  as long as agents are present, and amount should be directly given by the number of agents.
     * */
    void update(){
        if (cleanEveryStep)cleanContamination();
        else contaminationLevel*=exp(-fractionalDecrement);
    }
    /** Function to show the current status of a place - use with caution if there are many thousands of places! */
    void show(bool);//defined below once agents are defined
    //Because of the forward declaration of class agent, the full definition of this function has to wait until after the agent class is completed
};
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
    int counter=0;
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
    /** initialise the travel schedule  - this sets upt he list of places that will be visited, in order */
    void initTravelSchedule();
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
        //if (ID==0)std::cout<<"at Home "<<std::endl;
        
    }
    /** do any things that need to be done at work */
    void atWork(){
        //if (ID==0)std::cout<<"at Work"<<std::endl;
    }
    /** do any things that need to be done while travelling */
    void inTransit(){
        //if (ID==0)std::cout<<"on Bus"<<std::endl;
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//needed here for places as agents are pre-declared before place class
/** @param listAll Optional argument to show every place - only useful if there are just a few! */
void place::show(bool listAll=false){
    std::cout<<"Place ID "<<ID<<" has "<<occupants.size()<<" occupants"<<std::endl;
    if (listAll){
        std::cout<<"List of Occupant IDs:- "<<std::endl;
        for (auto o : occupants){
            std::cout <<o->ID<<std::endl;
        }
    }
}
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
class travelSchedule{
    /** A vector of named integers that holds the default travel destinations in the order that places will be visited \n
     * The names are taken from the agent class, where and enum defines the names \n
     * Note that vector indices start at 0
     */
    std::vector<agent::placeTypes> destinations; 
    /** For each place there is a time spent at each location (in hours at the moment), held here
     *  A short integer, so only up to 255 placeTypes can be held  in one schedule at present!
     */
    std::vector<short>timeSpent;
    /** The name of the destination currently pointed at by this travel schedule */
    agent::placeTypes currentDestination;
    /** An index into destinations and/or timeSpent vector for the current destination */
    int index;
public:
    /** @brief Constructor to build the schedule 
     *  add the placeTypes to be visited in order to the destinations vector, and the corresponding time that will be spent in each place to the timeSpent vector. \n
     *  schedule time in a given place should be an integer nultiple of the timestep.\n
     *  Index is set to point to the last place so that a call to getNextLocation will run the schedule back to the top.
     */
    travelSchedule(){
        destinations.push_back(agent::vehicle);//load people into busstop (or transportHub) rather than direct into bus? - here they would wait
        timeSpent.push_back(1);
        destinations.push_back(agent::work);//trip chaining? how to handle trips across multiple transport hubs? how to do schools (do parents load up childer?) and shops? - use a stack to modify default schedule!
        timeSpent.push_back(8);
        destinations.push_back(agent::vehicle);
        timeSpent.push_back(1);
        destinations.push_back(agent::home);
        timeSpent.push_back(14);
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
    short getTimeAtCurrentPlaceInHours(){
        return timeSpent[index];
    }

};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//defined here so as to be after travelSchedule
void agent::update()
{
        //Use the base travel schedule - initialised at home for everyone
        counter--;
        if (counter==0){
            currentPlace=schedule->getNextLocation();
            counter=schedule->getTimeAtCurrentPlaceInHours();
        }
        //expensive - only needed if agents need direct agnt-to-agent interactions in a place -
        //moveTo(currentPlace);
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
}
void agent::initTravelSchedule(){       
   schedule=new  travelSchedule();
   currentPlace=schedule->getNextLocation();
   counter=schedule->getTimeAtCurrentPlaceInHours();
}
void agent::cough()
{
        //breathInto(place) - scales linearly with the time spent there (using uniform timesteps) - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        if (diseased) places[currentPlace]->increaseContamination(disease::shedInfection());
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//include the model header file here so it knows the definitions of agent/place etc.
#include "model.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** set up and run the model 
 @param argc The number of command line arguments - at the moment only 1 can be handled
 @param argv The argument values - expected to be just the name of the parameter file */
int main(int argc, char **argv) {


    std::cout<<"Model version 0.2"<<std::endl;

    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set started at: "<<ctime(&t)<<std::endl;
    //time the whole set of runs
    auto startSet=timeReporter::getTime();
    //set up the parameters using an optional command-line argument
    //first set defaults
    parameterSettings parameters;
    parameters.setParameter("model.version","v0.2");
    //now read from a file
    if (argc ==1){
        std::cout<<"Using default parameter file"<<std::endl;
        parameters.readParameters("../defaultParameterFile");
    }else{
        std::cout<<"Default parameter file overridden on command line"<<std::endl;
        parameters.readParameters(argv[1]);
    }
    //set the number of OMP threads used to parallelise loops
    omp_set_num_threads(parameters.get<int>("run.nThreads"));

    //make sure there is at least one run!
    if (parameters.get<int>("run.nRepeats")<=0){
        parameters.setParameter("run.nRepeats","1");
    }
    //initialise the disease
    disease d(parameters);
    //repeat the model run nRepeats times with different random seeds
    int seed=parameters.get<int>("run.randomSeed");
    int increment=parameters.get<int>("run.randomIncrement");
    
    for (int runs=0;runs<parameters.get<int>("run.nRepeats");runs++){
        std::time_t tr=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        std::cout<<"Run repeat number: "<<runs+1<<" started at"<<ctime(&tr)<<std::endl;
        //increment the random seed value - note intially runs=0, so the default seed gets used
        parameters.setParameter("run.randomSeed",std::to_string(seed+runs*increment));
        //If number of runs>1, clear out the run number so that it will get set to auto-increment on each run.
        //This is needed to get the directory structure right - see model::setOutputFilePaths
        if(runs>0)parameters.setParameter("experiment.run.number","");
        //create and initialise a new model
        //any variations to parameter values should happen before this, so that the values
        //get properly saved to the output RunParameters file (created by model::setOutputFilePaths)
        model m(parameters);
        //start a timer to record the execution time
        auto start=timeReporter::getTime();
        //loop over time steps
        for (int step=0;step<parameters.get<int>("run.nSteps");step++){
            if (step%100==0)std::cout<<"Start of step "<<step<<std::endl;
            m.step(step,parameters);
        }
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Execution time after initialisation: ",start,end);
        t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        std::cout<<"Run finished at: "<<ctime(&tr)<<std::endl;
    }
    //report overall time taken
    auto endSet=timeReporter::getTime();
    timeReporter::showInterval("Total execution time for all runs: ",startSet,endSet);
    //work out the finish time
    t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set finished at: "<<ctime(&t)<<std::endl;
    return 0;
}



