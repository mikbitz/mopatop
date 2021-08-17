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
#include <iostream>
#include<algorithm>
#include <random>
#include <map>
#include<fstream>
#include<vector>
#include<chrono>
#include<ctime>
#include<set>
#include<string>
#include<assert.h>
#include<omp.h>
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** 
 * @brief Simple static class to abstract the clunky C++ chrono system
 * @details This class allows a variable to be created that will store the current run-time at the point it is created. \n
 * A second call later on then allows for the elapsed time to be calculated and show with the \ref showInterval method. \n
 * For example:-
 * \code
 * auto start=timeReporter::getTime();
 * ... do some stuff ...
 * auto end=timeReporter::getTime();
 * showInterval("time taken was",start,end);
 * \endcode
 * Note the use of "auto" so that the datatype of start and end doesn't need to be remembered!
 */
class timeReporter{
public:
    /** get the time now */
    static std::chrono::time_point<std::chrono::steady_clock> getTime(){return std::chrono::steady_clock::now();}
    /** show the interval between two time points in seconds (to the nearest millsecond) using standard output to the terminal
     @param s A string containing the message to show describing this time interval
     @param start The start of the interval as reported by \ref getTime
     @param end The correspinding end of the interval
     */
    static void showInterval(std::string s,std::chrono::time_point<std::chrono::steady_clock> start,std::chrono::time_point<std::chrono::steady_clock> end){
        std::cout<<s<<float(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())/1000<<" seconds"<<std::endl;
    }

};
//------------------------------------------------------------------------
/**
 * @brief Set up a wrapper class that will provide uniform pseudo-random numbers between 0 and 1 \n
 * @details As usual with random number generators, the sequence is actually periodic iwth a very long period.
 * By setting the seed one can generate the same sequence repeatedly by using the same seed.
 * The C++ random number generators are rather complicated. This class allows selection of one of the available generators \n
 * without needing to look up the detail. It generates a random double between 0 and 1 using the mersenne twister generator. \n
 * The seed defaults to 0 but can be reset with \ref setSeed - note this seems to need the twister to be completely re-created, hence the use of a pointer
 * Example:-
 * \code
 * randomizer r=randomizer::getInstance(0);
 * r.setSeed(12991);
 * double randomvalue=r.number();
 * \endcode
 * Uses a  map to define multiple independent generators, so there is only one random sequence across all agents for a given value of the short integer parameter i 
 * The idea is that in a multi-threaded application, each thread can have its own RNG - although note there is only one instance of twister (and hence the seed) for all.
 * Using the thread number leads to a small improvement in speed.
*/
class randomizer {
public:
    /** @brief get a reference to one of a set of random number generators.
     * If no appropriate instance yet exists, create it. \n
     * Instances are currently parameterized by omp thread number by calling omp_get_thread_num()\n
     * (which returns zero if only one thread)
     * @return A reference to the available instance
     * */
    static randomizer& getInstance(){
        short i=omp_get_thread_num();
        if (instance[i]==nullptr){
            instance[i]=new randomizer();
        }
        return *instance[i];
    }
    /** The distribution to be generated is uniform from 0 to 1 */
    std::uniform_real_distribution<> uniform_dist;
    /**  Use mersenne twister with fixed seed as the random number engine */
    std::mt19937* twister;
public:
    //~randomizer(){
    //  clean();
   // }
    /** @brief return the next pseudo-random number in the current sequence */
    double number(){
     return uniform_dist(*twister);
    }
    /** Set the seed that starts off a given random sequence 
     *param s The starting integer - any value can be used that fits with the size of int*/
    void setSeed(int s){
        std::cout<<"randomizer seed set to "<<s <<std::endl;
        delete twister;
        twister=new std::mt19937(s);
    }
private:
    /** The instance of this class. As this is a singleton (there can only ever be one of this class anywhere in the code) the actual instance is hidden from the user of the class */
    static std::map<short,randomizer*> instance;
    /** The constructor makes the class instance - again private so that access can be controlled. The class is used through the getInstance method */
    randomizer(){
        uniform_dist=std::uniform_real_distribution<> (0,1);
        std::cout<<"A randomizer was set up with seed 0" <<std::endl;
        twister=new std::mt19937(0);
    }
    /** if needed, clean up the pointer - however, given there is only one, it will just get deleted at end of program execution. */
    //void clean(){if (instance!=nullptr) {delete instance;instance=nullptr;}}
};
//------------------------------------------------------------------------
//static class members have to be initialized
std::map<short,randomizer*> randomizer::instance;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief ParameterSettings is a class designed to hold all the parameters for the model
 * @details At present these are hard coded here, but more usefully these could be delegated to an input file.
 * At the moment all parameters are strings, so need to be converted at point of use - \n
 * however, returning different data types depending on parameter name is a little tricky...
*/
class parameterSettings{
const std::type_info& ti1 = typeid(int);
    /** @brief a map from parameter names to parameter values, all currently strings */
    std::map<std::string,std::string>parameters;
    /** @brief function to check whether a requiested parameter name exists.\n
        If not exit the program.
        */
    bool is_valid(std::string s){
        auto it = parameters.find(s);
        //check that the parameter exist
        if(it == parameters.end()){
            std::cout<<"Invalid parameter: "<<s<<std::endl ;
            exit(1);
        }
        return true;
    }
public:
    parameterSettings(){

    //total time steps to run for
    parameters["nSteps"]="4800";
    //number of agents to create
    parameters["nAgents"]="600";
    //number of OMP threads to use increase the number here if using openmp to parallelise any loops.
    //Note number of threads needs to be <= to number of cores/threads supported on the local machine
    parameters["nThreads"]="1";
    //random seed
    parameters["randomSeed"]="0";
    //path to the output file
    parameters["outputFile"]="diseaseSummary.csv";
    }
//------------------------------------------------------------------------
/** @brief allow parameters to be returned using an instance of class parameters using a string
 *  @param s the name of the parameter requested.
 *   Example:-
 * \code
 * parameterSettings p;
 * std::string filename=p("outputFile");
 * \endcode
 * function fails if the requested parameter not been defined - the program halts\n
 * This version returns only the string variant of the parmeter value
 */
    std::string operator ()(std::string s){
        is_valid(s);
        return parameters[s];
    }
//------------------------------------------------------------------------
/** @brief allow parameters to be returned with a given type conversion, in this case double\n
 *  @param s the name of the parameter requested.
 *  @details Since the parameters are stored as strings, but may represent other types, the get function allows\n
 *  them to be converted safely, with different function versions for each possble datatype
 *   Example:-
 * \code
 * parameterSettings p;
 * std::string rate=p.get<double>("rate");
 * \endcode
 * function fails if the requested parameter not been defined - the program halts\n
 */
    template <typename T>
    typename std::enable_if<std::is_same<T,double>::value, T>::type
    get(std::string s){
        is_valid(s);
        return stod(parameters[s]);
    }
//------------------------------------------------------------------------
/** @brief Specialisation of get for floats\n
 *  @param s the name of the parameter requested.
 */
    template <typename T>
    typename std::enable_if<std::is_same<T, float>::value, T>::type
    get(std::string s){
        is_valid(s);
        return stof(parameters[s]);
    }
//------------------------------------------------------------------------
/** @brief Specialisation of get for int\n
 *  @param s the name of the parameter requested.
 */
    template <typename T>
    typename std::enable_if<std::is_same<T, int>::value, T>::type
    get(std::string s){
        is_valid(s);
        return stoi(parameters[s]);
    }
//------------------------------------------------------------------------
/** @brief Specialisation of get for long integers\n
 *  @param s the name of the parameter requested.
 */
    template <typename T>
    typename std::enable_if<std::is_same<T, long>::value, T>::type
    get(std::string s){
        is_valid(s);
        return stol(parameters[s]);
    }
//------------------------------------------------------------------------
/** @brief Specialisation of get for unsigned integers\n
 *  @param s the name of the parameter requested.
 */
    template <typename T>
    typename std::enable_if<std::is_same<T, unsigned>::value, T>::type
    get(std::string s){
        is_valid(s);
        //seems there is no conversion just to unsigned, so use unsigned long
        return stoul(parameters[s]);
    }
//------------------------------------------------------------------------
/** @brief The default get just returns the string value\n
 *  @param s the name of the parameter requested.
 */
        std::string get(std::string s){
        is_valid(s);
        return parameters[s];
    }
};
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
/**
 * @brief Simple static class to represent a very very simple disease
 * @details Use of static class allws some of the details of the disease to be abstracted here, without needed a separate disease object in every agent \n
 * The disadvantage of this is that the parameters are fixed outside teh class definition (a requiremnet of C++ for static (i.e. class-based rather than instance-based) variables) \n
 * @todo update this so that parameters can be set, e.g. from a parameter file
*/
class disease{
    /** fixed per timestep chance of recovery */
    static float recoveryRate;
    /** fixed per timespte contribution to contamination at a site */
    static float infectionShedLoad;
public:
    /** recover with a fixed chance in a given timestep - this function needs to be called every timestep by infected agents*/
    static bool recover (){
      if (recoveryRate>randomizer::getInstance().number())return true;else return false;
    }
    /** contract disease if contamination is large enough (note it could be >1) - again called very time step */
    static bool infect(float contamination){
      if (contamination >randomizer::getInstance().number()) return true; else return false;
    }
    /** contribute infection to the place if diseased - called every timestep by infected agents */
    static float shedInfection(){return infectionShedLoad;}

};
float disease::recoveryRate=0.0008;
float disease::infectionShedLoad=0.001;
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


    std::cout<<"Model version -0.9"<<std::endl;
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
 * (or else it can be reset to zero at the start of each step - disease transmission then just depends on number of agents in any location)\n
 * The release of total contamination depends linearly on time spent in each location. \n
 * In a contaminated location, susceptible agents can pick up the infection - they then recover with a fixed chance per timestep, and are subsequently immune.
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
 * as suggested in Grimm et al https://doi.org/10.1016/j.ecolmodel.2010.08.019 and extended by Muller et al https://doi.org/10.1016/j.envsoft.2013.06.003\n 
 * @section over Overview
 * @subsection purp Purpose
 * @subsection enti Entities, State variables and Scales
 * @subsection proc Process Overview and Scheduling
 * @section desi Design concepts
 * @subsection basi Basic Principles
 * @subsection theo Theoretical and Empirical Background
 * @subsection indiv Individual Decision Making
 * @subsection lear Learning
 * @subsection indis Individual sensing
 * @subsection indip Individual prediction
 * @subsection objec Objectives
 * @subsection inte Interaction
 * @subsection colle Collectives
 * @subsection emer Emergence
 * @subsection adap Adaptation
 * @subsection hete Heterogeneity
 * @subsection stoc Stochasticity
 * @subsection obse Observation
 * @section imple Implementation Details
 * @subsection init Initialisation
 * @subsection inpu Input Data
 * @subsection subm Submodels
 * @subsubsection cont Contamination
 * Places have a contamination level which is added to when agents local to the place cough. The level then decreases at a fixed rate over time.\n
 * \f$\frac {dc}{dt} = \sum_{agents}shedInfection-fractionalDecrement * c\f$
 */


