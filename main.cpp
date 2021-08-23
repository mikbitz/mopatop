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
    /** Rate of decrease of contamination - per time step \n */
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
     *  \ref cleanEveryStep is set, the place has all contamination removed - useful if caontamination shoudl only be present\n
     *  as long as agents are present, and amount should be directly given by the number of agents.
     * */
    void update(){
        if (cleanEveryStep)cleanContamination();
        else contaminationLevel*=fractionalDecrement;
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
    long nAgents;
    /** A string containing the file path for output, for a given experiment, to be put before specific file names */
    std::string _filePrefix;
    /** A string containing any extra default characters to come after the filename */
    std::string _filePostfix;
    /** The output file stream */
    std::ofstream output;
public:
    /** Constructor for the model - set up the random seed and the output file, then call \ref init to define the agents and the places \n
        The time reporter class is used to check how long it takes to set up everything \n
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied*/
    model(parameterSettings& parameters){
        randomizer r=randomizer::getInstance();
        nAgents=parameters.get<long>("run.nAgents");
        r.setSeed(parameters.get<int>("run.randomSeed"));
        //create the directories and paths for the current experiment
        setOutputFilePaths(parameters);
        //output file
        output.open(_filePrefix+parameters("outputFile")+_filePostfix+".csv");
        //header line
        output<<"step,susceptible,infected,recovered"<<std::endl;
        //Initialisation can be slow - check the timing
        auto start=timeReporter::getTime();
        init(parameters);
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Initialisation took: ", start,end);
    }
    //------------------------------------------------------------------------
    /** @brief destructor - make sure output files are porperly closed */
    ~model(){
        output.close();
    }
    //------------------------------------------------------------------------
    /** @brief Create the system of directories for model experiments and their outputs
     *  @details A system of directories gets created, starting from the top level experiment.output.directory. \n
     *  To this is added the name of an experiment, which is assumed to consist of a number of separate runs with \n
     *  different parameter values (e.g. random seed). By default a max. of 10000 runs is assumed - this gives tidy names\n
     *  like ./output/experiment.test/run_0000 ./output/experiment.test/run_0001 ... ./output/experiment.test/run_9999 \n
     *  More then 10000 will be ok, but the directory names will spill over to ./output/experiment.test/run_10000 etc. \n
     *  The number can be customised using experiment.run.prefix, which should be set to a power of 10.\n
     *  A single run can be specified by setting experiment.run.number - if this coincides with and existing run/directory\n
     *  then the files there may be overwritten (unless explicit output file names are changed).
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied*/
    void setOutputFilePaths(parameterSettings& parameters){
        //naming convention for output files
        _filePrefix=   parameters.get("experiment.output.directory")+"/experiment."+parameters.get("experiment.name");
        if (!std::filesystem::exists(_filePrefix))std::filesystem::create_directories(_filePrefix);
        std::string runNumber= parameters.get("experiment.run.number");
        std::string m00="/run_";
        if (runNumber!=""){
            m00=m00+runNumber;
        }else{
            //auto-increment run number if run.number is not set
            int i=0;
            //allow for userdefined number of total runs - expected to be a power of ten
            std::string zeros=parameters("experiment.run.prefix");
            //add just the zero part (strip off expected leading 1)
            m00="/run_"+zeros.substr(1);
            // Find a new directory name - then next in numerical order
            while(std::filesystem::exists(_filePrefix+m00)){    
                i++;
                std::stringstream ss;
                int zeroIndex=stoi(zeros)/10;while(zeroIndex>i && zeroIndex>1){ss<<"0";zeroIndex=zeroIndex/10;}
                ss<<i;
                runNumber=ss.str();
                m00="/run_"+runNumber;
            }
        }
        if (!std::filesystem::exists(_filePrefix+m00))std::filesystem::create_directories(_filePrefix+m00);
        parameters.setParameter("experiment.run.number",runNumber);
        _filePrefix= _filePrefix+m00+"/";
        _filePostfix="";
        std::cout<<"Outputfiles will be named "<<_filePrefix<<"<Data Name>"<<_filePostfix<<".<filenameExtension>"<<std::endl;
        parameters.saveParameters(_filePrefix);
    }
    //------------------------------------------------------------------------
    /** @brief Set up the agents and places, and allocate agents to homes, workplaces, vehicles. \n
     *  @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied
     *  @details The relative structure of the places, size homes and workplaces and the number and size of transport vehicles, together with the schedule, \n
     *  will jointly determine how effective the disease is a spreading, given the contamination rate and recovery timescale \n
     *  This simple intializer puts three agents in each home, 10 agents in each workplace and 30 in each bus - so agents will mix in workplaces, home and buses in slightly different patterns.
     */
    void init(parameterSettings& parameters){
        //create homes - one third of the agent number
        for (int i=0;i<nAgents/3;i++){
            place* p=new place(parameters);
            places.push_back(p);
            places[i]->setID(i);
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
            place* p=new place(parameters);
            places.push_back(p);
            places[i]->setID(i);
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
            place* p=new place(parameters);
            places.push_back(p);
            places[i]->setID(i);
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
    //------------------------------------------------------------------------
    /** @brief Advance the model time step \n
    *   @details split up the timestep into update of places, contamination of places by agents, infection and progress of disease and finally update of agent locations \n
        These loops are separated so they can be individually timed and so that they can in principle be individually parallelised with openMP \n
        Also to avoid any systematic biases, agents need to all finish their contamination step before any can get infected. 
        @param stepNumber The timestep number passed in from the model class
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied*/
    void step(int stepNumber, parameterSettings& parameters){
        //set some timers so loop relative times can be compared - note disease loop tends to get slower as more agents get infected.
        auto start=timeReporter::getTime();
        auto end=start;
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
/** set up and run the model 
 @param argc The number of command line arguments - at the moment only 1 can be handled
 @param argv The argument values - expected to be just the name of the parameter file */
int main(int argc, char **argv) {


    std::cout<<"Model version 0.2"<<std::endl;

    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set started at: "<<ctime(&t)<<std::endl;
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
            if (step%1000==0)std::cout<<"Start of step "<<step<<std::endl;
            m.step(step,parameters);
        }
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Execution time after initialisation: ",start,end);
        t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        std::cout<<"Run finished at: "<<ctime(&tr)<<std::endl;
    }
    //work out the finish time
    t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set finished at: "<<ctime(&t)<<std::endl;
    return 0;
}



