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
public:
    /** Unique identifier for a place - should be able to go up to about 4e9 */
    unsigned long ID;
    /** An arbitrary number giving how infectious a given place currently might be - needs calibration to get a suitable per-unit-time value. \n 
     One might expect it to vary with the size of a given location
     */
    float contaminationLevel;
    /** Rate of decrease of contamination - per time step \n */
    float fractionalDecrement;
    /** unique list of current people in this place - intended for direct agent-agent interaction \n
     * For the current disease model this is not needed, as the agents need only know where they are to contaminate a place \n
     * currently this is not used...seems to add about 20% to memory requirement if populated. \n
     * A std::set is unique - so the same agent can be added many times but will only apear once.\n
     * The set uses the pointer to a given agent as the key, so its easy to insert or remove arbitrary agents */
    std::set<agent*> occupants;
    /** set up the place. Assumed initially clean. The decrement vlaue might very with place type and ventialtion level... */
    place(){
        ID=0;
        contaminationLevel=0.;
        fractionalDecrement=0.1;        
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
    void increaseContamination(float amount){
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
    float getContaminationLevel(){return contaminationLevel;}
    /** The contamination in each place decays exponentially. This function shoudl be called every (uniform) time step \n
     *  This way places without any currently infected agents gradually lose their infectiveness
     * */
    void update(){contaminationLevel*=fractionalDecrement;}
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
    void disease(){
        //recovery
        if (diseased){
            if (disease::recover()) {diseased=false ; immune=true;}
        }
        //infection
        if (disease::infect(places[currentPlace]->getContaminationLevel()) && !immune)diseased=true;
        //immunity loss could go here...
    }
    /** do any things that need to be done at home */
    void atHome(){
        if (ID==0)std::cout<<"at Home"<<std::endl;
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
     *  add the placeTypes to be visited in order to the destinations vector, and the corresponding time that will be spent in each place to the timeSpent vector \n
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
/**
 * @brief The model contains all the agents and places, and steps them through time
 * @details At the moment time steps are not even in length (since the schedule values are not of the same length for work/home/transport)\n

*/
class model{
    /** A container to hold pointers to all the agents */
    std::vector<agent*> agents;
    /** A container to hold the places */
    std::vector<place*> places;
    /** The number of agents to be created */
    int nAgents;
    /** The output file stream */
    std::ofstream output;
public:
    /** Constructor for the model - set up the random seed and the output file, then call \ref init to define the agents and the places \n
        The time reporter class is used to check how long it takes to set up everything \n
        @param parameters A class that holds all the possible parameter settings for the model */
    model(parameterSettings parameters){
        randomizer r=randomizer::getInstance();
        nAgents=parameters.get<int>("nAgents");
        r.setSeed(parameters.get<int>("randomSeed"));
        //output file
        output.open(parameters("outputFile"));

        //header line
        output<<"step,susceptible,infected,recovered"<<std::endl;
        //Initialisation can be slow - check the timing
        auto start=timeReporter::getTime();
        init(parameters);
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Initialisation took: ", start,end);
    }
    /** @brief Set up the agents and places, and allocate agents to homes, workplaces, vehicles. \n
     * @param parameters A class that holds all the possible parameter settings for the model.
     * @details The relative structure of the places, size homes and workplaces and the number and size of transport vehicles, together with the schedule, \n
     *  will jointly determine how effective the disease is a spreading, given the contamination rate and recovery timescale \n
     * This simple intializer puts three agents in each home, 10 agents in each workplace and 30 in each bus - so agents will mix in workplaces, home and buses in slightly different patterns.
     */
    void init(parameterSettings parameters){
        //create homes - one third of the agent number
        for (int i=0;i<nAgents/3;i++){
            place* p=new place();
            places.push_back(p);
            places[i]->ID=i;
        }
        //allocate 3 agents per home
        for (int i=0;i<nAgents;i++){
            agent* a=new agent();
            agents.push_back(a);
            agents[i]->ID=i;
            agents[i]->setHome(places[i/3]);
        }
        //create work places - one tenth as many as agents - add them on to the end of the place list.
        for (int i=nAgents/3;i<nAgents/3+nAgents/10;i++){
            place* p=new place();
            places.push_back(p);
            places[i]->ID=i;
        }
        //shuffle agents so household members get different workplaces
        random_shuffle(agents.begin(),agents.end());
        //allocate 10 agents per workplace
        for (int i=0;i<agents.size();i++){
            assert(places[i/10+nAgents/3]!=0);
            agents[i]->setWork(places[i/10+nAgents/3]);
        }
        //create buses - one thirtieth since 30 agents per bus. add them to the and of the place list again
        for (int i=nAgents/3+nAgents/10;i<nAgents/3+nAgents/10+nAgents/30;i++){
            place* p=new place();
            places.push_back(p);
            places[i]->ID=i;
        }
        //allocate 30 agents per bus - since agents aren't shuffled, those in similar workplaces will tend to share buses. 
        for (int i=0;i<agents.size();i++){
            assert(places[i/30+nAgents/3+nAgents/10]!=0);
            agents[i]->setTransport(places[i/30+nAgents/3+nAgents/10]);
        }
        //set up travel schedule - same for every agent at the moment - so agents are all on the bus, at work or at home at exactly the same times
        for (int i=0;i<agents.size();i++){
            agents[i]->initTravelSchedule();
        }
        //report intialization to std out 
        std::cout<<"Built "<<agents.size()<<" agents and "<<places.size()<<" places."<<std::endl;
        //set off the disease! - one agent is infected at the start.
        agents[0]->diseased=true;
    }
    /** @brief Advance the model time step \n
    *   @details split up the timestep into update of places, contamination of places by agents, infection and progress of disease and finally update of agent locations \n
        These loops are separated so they can be individually timed and so that they can in principle be individually parallelised with openMP \n
        Also to avoid any systematic biases, agents need to all finish their contamination step before any can get infected. 
        @param stepNumber The timestep number passed in from the model class
        @param parameters A class that holds all the possible parameter settings for the model.*/
    void step(int stepNumber, parameterSettings parameters){
        //set some timers so loop relative times can be compared - note disease loop tends to get slower as more agents get infected.
        auto start=timeReporter::getTime();
        auto end=start;
        if (stepNumber%10==0)std::cout<<"Start of step "<<stepNumber<<std::endl;
        //update the places - changes contamination level
        if (stepNumber==0)start=timeReporter::getTime();
        //note the pragma statement here allows openmp to parallelise this lopp over several threads 
        #pragma omp parallel for
        for (int i=0;i<places.size();i++){
            places[i]->update();
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Time updating places: ",start,end);
            start=end;
        }
        //counts the totals
        int infected=0,recovered=0;
        //do disease - synchronous update (i.e. all agents contaminate before getting infected) so that no agent gets to infect ahead of others.
        //alternatively could be randomized...depends on the idea of how a location works...places could be sub-divided to mimic spatial extent for example.
        #pragma omp parallel for
        for (int i=0;i<agents.size();i++){
            agents[i]->cough();
        }
        if(stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time coughing: ",start,end);
            start=end;
        }
        //the disease progresses
        #pragma omp parallel for
        for (int i=0;i<agents.size();i++){
            agents[i]->disease();
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time being diseased: ",start,end);
            start=end;
        }
        //accumulate totals
        for (int i=0;i<agents.size();i++){
            if (agents[i]->diseased)infected++;
            if (agents[i]->immune)recovered++;
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time on accumulating disease totals: ",start,end);
            start=end;
        }
        //move around, do other things in a location
        #pragma omp parallel for
        for (int i=0;i<agents.size();i++){
            agents[i]->update();
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time updating agents: ",start,end);
            start=end;
        }
        //output a summary .csv file
        output<<stepNumber<<","<<agents.size()-infected-recovered<<","<<infected<<","<<recovered<<std::endl;
        //show the step number every 10 steps
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time on file I/O: ",start,end);
        }
      
        //show places - just for testing really so commented out at present
        for (int i=0;i<places.size();i++){
            //places[i]->show();
        }
    }
    
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** set up and run the model */
int main(int argc, char **argv) {


    std::cout<<"Model version 0.1"<<std::endl;
    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run started at: "<<ctime(&t)<<std::endl;
    parameterSettings parameters;

    //create and initialise the model
    model m(parameters);

    omp_set_num_threads(parameters.get<int>("nThreads"));
    
    //start a timer to record the execution time
    auto start=timeReporter::getTime();
    //loop over time steps
    for (int step=0;step<parameters.get<int>("nSteps");step++){
        m.step(step,parameters);
    }
    auto end=timeReporter::getTime();
    timeReporter::showInterval("Execution time after initialisation: ",start,end);
    return 0;
}
/**
 * @mainpage
 * @section intro_sec Introduction
 * This model is aimed at representing the patterns of movement and interaction of agents that represent individual people as they go about their daily activities. \n
 * The current version is intended to show how this can be done using a simple C++  program (using this language for speed of execution) \n
 * For this reason at the moment all code is in a single file. This can become unwieldy in a larger application though. \n
 * 
 * The current objective is to be able to model a simple disease, and to tie this to agent behaviour at the scale of an entire country.\n
 * Loop parallelisation with openMP is used to accelerate execution if needed.
 * @subsection Main Main ideas
 * Agents move between places according to a given fixed travel schedule. Places include transport vehicles. \n
 * In each place, agents with a disease can add contamination, which then decays exponentially over time \n
 * (or else it can be reset to zero at the start of each step - disease transmission then just depends on \n
 * number of agents in any location). The release of total contamination depends linearly on time spent in \n
 * each location. In a contaminated location, susceptible agents can pick up the infection - they then recover\n
 * with a fixed chance per timestep, and are subsequently immune.
 * @subsection Compiling Compiling the model
 * On a linux system with g++ installed just do \n
 * g++ -o agentModel -O3 -fopenmp main.cpp\n
 * If using openmp (parallelised loops) then set the  number of threads in the parameterSettings class.\n
 * Note the number of cores to be used must be <= number supported by the local machine!
 * @subsection Run Running the model
 * At present this is a simple command-line application - just type the executable name (agentModel above) and then return.\n
 * @subsection dett Detailed Description
 * For a formalised description of the model  see  @ref ODD
 * @page ODD ODD description
 * @section odd_intro Introduction
 * This page describes the model in more detail using the ODD+D formalism (Overview, Design Concepts and Details + Decisions) \n
 * as suggested in Grimm et al https://doi.org/10.1016/j.ecolmodel.2010.08.019 \n
 * and extended by M&uuml;ller et al https://doi.org/10.1016/j.envsoft.2013.06.003\n 
 * @section over Overview
 * This is an agent-based model of movement and respiratory infectious disease-spread.
 * @subsection purp Purpose
 * To model the spread of disease through a population of individual people as they move between \n
 * the places where they do their daily activites.The model is intended to scale to large numbers \n
 * of agents, at least the scale of a medium sized country, without requiring too much run-time.
 * @subsection enti Entities, State variables and Scales
 * @subsection sp space
 * The model currently has no notion of spatial scale, except as implicitly represented by time \n
 * spend in vehicles when travelling between places, Individual places have no internal spatial\n
 * structure - they are assumed completely mixed, so that all agents a can in principle access all others.
 * @subsection ti time
 * There is a fixed time step, representing one hour of real-world time. Every place and agent and disease \n
 * is updated synchronously at each step. This ensures that a)Updates to contamination take place regularly\n
 * even if there are no agents present b)Agents do not need to co-ordinate in terms of times spent in places\n
 * (which otherwise require o(n squared) interactions in each place), and can meet by chance within a timestep\n
 * c)outputs are easily set up at regular times. Care is required to make sure this does not lead to agents\n
 * preferentially always "acting first", so may either require the order of agents activity to be randomised, or\n
 * for particular activities to be completed for ALL agents before subsequent ones (e.g.all agents need to complete\n
 * coughing in a place before any are allowed to update their infection status)\n
 * However, agents also have an event-based schedule - so the regular timestep does not preclude agents having \n
 * heterogenous activities with respect to travel or other actions, provided that the schedule time-grain is not shorter\n
 * than the main timestep (or the schedule has its own sub-time steps for taking forward an activity).
 * @subsection rd random numbers
 * A single pseudo random number sequence is generated using the mersenne twister algorithm
 * @subsection tr benchmarking
 * A simple \ref timeReporter class is set up that can be used to report the run-time of different parts of the model\n
 * Useful for benchmarking and understading which parts are the most comu[puationally expensive.
 * @subsection pa parameters
 * All of the model parameters are currently hard coded, but (except for disease/contamination)\n
 * They are all collected into a single \ref parameterSettings class - this includes things like\n
 * random seed, number of agents, number of steps to run for.
 * @subsection pl places
 * Places are at present simple containers that can take any number of agents. Agents keep a flag pointing to their\n
 * current place, so that they can add contamination, or examine the contamination level inorder to become infected.\n
 * Places are also containers for the contamination, and evolve its level over time.\n
 * In a more complex model places could have substructure (e.g. a canteen, multiple buildings in a hospital) and a \n
 * topology (which place is next to which other, are there floors with differnt access) and geometry (how far away are \n
 * other places).\n
 * At present there are three types of place - home, work and vehicle - others could be added (e.g. shops, hospitals).
 * @subsection ag agents
 * Agents are individual people with just a few attributes - their current place, whether they are infected or immune, \n
 * a travel schedule, and a count-down to the next travel-schedule event. They also have a list of known places \n
 * (home, work,vehicle) which can differ for each agent - in practice some of them will share a home, a workplace, or a \n
 * travel vehicle (e.g. a bus), and this allows for the spread of the disease.
 * @subsection di disease
 * A single simple disease allows agents interrogate it and discover whether they to recover at a hard-coded fixed average\n
 * rate, to be infected if there is contamination and to shed infection into a place, again at a hard-coded rate per unit time.\n 
 * @subsection ts travelSchedule
 * Each agent has their own copy of a travel schedule - at present, though, this is identical for every agent.\n
 * The schedule has an ordered list of place types representing destination for travel, and a time to be spent\n
 * in each place of a given type. Agents use the place type to select their own particular place to travel to \n
 * when the scheduled time in their current place has expired.
 * @subsection proc Process Overview and Scheduling
 * @subsection up update places
 * At the start of the timestep all places update their contamination level
 * @subsection cu cough
 * Once places are updated, all agents with disease shed contamination into their current place.
 * @subsection pds progress disease
 * After the contamination step completes, agents get the chance to recover, or to be infected if susceptible.
 * @subsection ua update agents
 * Other activities can now take place (placeholders functions exist for actions at home, at work or in transit)
 * @section desi Design concepts
 * @subsection basi Basic Principles
 * Infectious diseases are spread by individuals shedding infectious material into an environment.\n
 * In the case of respiratory diseases this is by coughing or sneezing, which distributes infectious \n
 * droplets into the space they currently occupy. Over time, if they stay in the same place, and it \n
 * is not well ventilated, then the infectiousness of the place may increase (especially if, for example, \n
 * air-conditioning keeps particles suspended). However, such particles will gradually settle out onto \n
 * surfaces, and then lose their infecitousness over time.Other susceptible individuals moving into the \n
 * contaminated space may become infected, possibly some time after the original infectious agent has left.\n
 * Once individuals are infected, they too can spread the disease, but they have a constant rate per unit\n
 * time possibility of recovering.Recovered individuals acquire some level of immunity to re-infection, that\n
 * eventually may cause the disease to disappear. \n
 * People travel between different locations during their daily activity, and these places may have \n
 * different occupancy levels, depending on the type of activity they represent.These places are in \n
 * many cases largely separated from other locations in terms of the possibility of spreading disease,\n
 * so act to an extent as closed environments in which disease can spread.Such places include collectively\n
 * shared transport, such as buses trains and cars (but not, for example, bicycles, except maybe tandems).\n
 * The spatial relationship of these locations only matters to the extent that this determines how long \n
 * people spend in transport vehicles, and what potential there is for moving between locations(e.g. \n 
 * whether there are nearby restaurants, coffee shops etc.)
 * @subsection theo Theoretical and Empirical Background
 * The model is related to the classical SIR compartmental disease models, but varies from these in that the \n
 * population is not assumed to be completely mixed (although one can emulate this by having a single place for\n
 * agents to occupy, and therbey check this model against a SIR model, since places in this model are currently\n
 * assumed completely mixed internally). Respiratory infectious diseases such as COVID-19 seem to depend not just\n
 * on the characteristics of individual, but what kind of space is occupied, how long people spend jointly in the\n
 * space, and how confined it is. 
 * @subsection indiv Individual Decision Making
 * Agents decide to move to a new place when the time in the current place expires (as set by the travel schedule).
 * @subsection lear Learning
 * None.
 * @subsection indis Individual sensing
 * Agents detect the level of contamination at the current location.
 * @subsection indip Individual prediction
 * None.
 * @subsection objec Objectives
 * Agents just follow a fixed travel schedule.
 * @subsection inte Interaction
 * Interaction is indirect through contamination levels in the various places
 * @subsection colle Collectives
 * Agents collectively occupy places such as work, home and transport, at different times.
 * @subsection emer Emergence
 * The progress of the disease emerges as a result of contamination of places and subsequent catching of infection by agents
 * @subsection adap Adaptation
 * None.
 * @subsection hete Heterogeneity
 * Agents differ by their home, work and transport places, and whether they currently have the disease.
 * Places differ by contamination level.
 * @subsection stoc Stochasticity
 * The initial work places for agents are assigned at random irresective of home address.\n
 * Disease infection and recovery are controlled by pseudo-random numbers.
 * @subsection obse Observation
 * Total numbers susceptible, infected and recovered are output to a csv file, along with the timestep
 * @section imple Implementation Details
 * @subsection init Initialisation
 * - First homes are created - enough for 3 agents per home.\n
 * - Agents are then created and allocated 3 to a home untilt the list of agents is exhausted.\n
 * - Work places are then created, enough for 10 agents per workplace.\n
 * - The agents list is shuffled randomly, then agents are allocated to work places 10 at a time in order.\n
 * - Vehicles are created representing buses, with a capacity of 30 agents. Agents are allocated to\n 
 * buses in the same order as to work places\n
 * - Travel schedules are initialised with Agents on the bus heading home - every agents has the same\n
 * schedule with the exact same time spent in each place.\n
 * - One agent (agent 0) is infected with the disease\n
 * @subsection inpu Input Data
 * None.
 * @subsection subm Submodels
 * @subsubsection dise Disease
 * - Agents are infected if contamination > a uniform random number between 0 and 1, and not immune\n
 * - Agents recover if infected and recovery rate > a uniform random number between 0 and 1 \n
 * - Agents add a fixed value \ref disease::infectionShedLoad to a place, if infected.
 * @subsubsection cont Contamination
 * - Places have a contamination level which is added to when agents local to the place cough.\n
 * - The level then decreases at a fixed rate over time.\n
 * \f$\frac {dc}{dt} = \sum_{agents}infectionShedLoad-fractionalDecrement * c\f$ \n
 * - Places can have contamination reset to zero - if this is done every timestep, but before\n
 * agents cough, then the level just becomes proportional to the number of agents in a place.
 * @subsubsection trvl travel
 * - Agents set a counter whenever they move, which is the time to be spent in the place the move to\n
 * - Every time step, the counter is decremented by the time step size. Once the counter reaches zero\n
 * agents query the travel schedule for the next place type to move to and then go there.\n
 * - Structure is included to allow agents to add and remove themsleves from a list of agents in each place\n
 * but at present this is not needed (and the add and remove process is computationally quite expensive).
 */


