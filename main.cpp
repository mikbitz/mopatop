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
 * This class allows a variable to be created that will store the current run-time at the point it is created. \n
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
 * As usual with rando mnumber generators, the sequence is actually periodic iwth a very long period.
 * By setting the seed one can generate the same sequence repeatedly by using the same seed.
 * The C++ random number generators are rather complicated. This class allows selection of one of the available generators \n
 * without needing to look up the detail. It generates a random double between 0 and 1 using the mersenne twister generator. \n
 * The seed defaults to 0 but can be reset with \ref setSeed
 * Example:-
 * \code
 * randomizer r=randomizer::getInstance();
 * r.setSeed(12991);
 * double randomvalue=r.number();
 * \endcode
 * Use a singleton so there is only one random sequence across all agents 
 * @todo (?works for llel execution with openMP??)
*/
class randomizer {
public:
    /** @brief get a reference to the random number generator.
     * If no instance yet exists, create it
     * @return A reference to the single available instance
     * */
    static randomizer& getInstance(){ 
        if (instance==NULL){
            instance=new randomizer();
        }
        return *instance;
    }
    /** The distribution to be generated is uniform from 0 to 1 */
    std::uniform_real_distribution<> uniform_dist;
    /**  Use mersenne twister with fixed seed as the random number engine */
    std::mt19937 twister;
public:
    //~randomizer(){
    //  clean();
   // }
    /** @brief return the next pseudo-random number in the current sequence */
    double number(){
     return uniform_dist(twister);
    }
    /** Set the seed that starts off a given random sequence 
     *param s The starting integer - any value can be used that fits with the size of int*/
    void setSeed(int s){
        std::cout<<"randomizer seed set to "<<s <<std::endl;
        twister.seed(s);
    }
private:
    /** The instance of this class. As this is a singleton (there can only ever be one of this class anywhere in the code) the actual instance is hidden from the user of the class */
    static randomizer* instance;
    /** The constructor makes the class instance - again private so that access can be controlled. The class is used through the getInstance method */
    randomizer(){
        uniform_dist=std::uniform_real_distribution<> (0,1);
        std::cout<<"A randomizer was set up with seed 0" <<std::endl;
        twister.seed(0);
    }
    /** if needed, clean up the pointer - however, given there is only one, it will just get deleted at end of program execution. */
    void clean(){if (instance!=nullptr) {delete instance;instance=nullptr;}}
};
//------------------------------------------------------------------------
//static class members have to be initialized
randomizer* randomizer::instance=NULL;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//Forward declaration of agent as they are needed in place class
class agent;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief Places can have a list of occupants, and store disease contamination
*/
class place{
public:
    int ID;
    float contaminationLevel,fractionalDecrement;
    //unique list of current people in this place
    //currently this is not used...seems to add about 20% to memory requirement
    std::set<agent*> occupants;

    place(){
        ID=0;
        contaminationLevel=0.;
        fractionalDecrement=0.1;        
    }
    void add(agent* a){
        occupants.insert(a);
    }
    void remove(agent* a){
        occupants.erase(a);
    }
    void increaseContamination(float amount){
        contaminationLevel+=amount;
    }
    float getContaminationLevel(){return contaminationLevel;}
    //contamination decays exponentially
    void update(){contaminationLevel*=fractionalDecrement;}
    void show(bool);//defined below once agents are defined
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief Simple static class to represent a very very simple disease
*/
class disease{
    static float recoveryRate;
    static float infectionShedLoad;
public:
    //recover with a fixed chance in a given timestep
    static bool recover (){
      if (recoveryRate>randomizer::getInstance().number())return true;else return false;
    }
    //contract disease if contamination is large enough (note it could be >1)
    static bool infect(float contamination){
      if (contamination >randomizer::getInstance().number()) return true; else return false;
    }
    static float shedInfection(){return infectionShedLoad;}

};
float disease::recoveryRate=0.002;
float disease::infectionShedLoad=0.1;
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class travelSchedule;
/**
 * @brief The main agent class - each agent represents one person
*/
class agent{
public:
    int ID;
    //transport vehicles are places, albeit moveable!
    enum placeTypes{home,work,vehicle};
    std::vector<place*>places;
    placeTypes currentPlace;

    //the default schedule - currently every agent has the same - needs modification...(singleton?)
    travelSchedule* schedule;
    //disease parameters
    bool diseased,immune;
    agent(){
        diseased=false;
        immune=false;
        //this has to be the same size as the placeTypes enum
        places.resize(3);
    }
    void moveTo(placeTypes location){
        places[currentPlace]->remove(this);
        places[location]->add(this);
        currentPlace=location;
    }
    void update();
    void initTravelSchedule();
    void cough();
    void disease(){
        //recovery
        if (diseased){
            if (disease::recover()) {diseased=false ; immune=true;}
        }
        //infection
        if (disease::infect(places[currentPlace]->getContaminationLevel()) && !immune)diseased=true;
        //immunity loss could go here...
    }
    void atHome(){
        //if (ID==0)std::cout<<"at Home"<<std::endl;
    }
    void atWork(){
        //if (ID==0)std::cout<<"at Work"<<std::endl;
    }
    void inTransit(){
        //if (ID==0)std::cout<<"on Bus"<<std::endl;
    }
    void setHome(place* pu){
        places[home]=pu;
        //start all agents at home - if using the occupants list, add to the home place
        //pu->add(this);
        currentPlace=home;
    }
    void setWork(place* pu){
        places[work]=pu;
    }
    void setTransport(place* pu){
        places[vehicle]=pu;
    }

};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//needed here for places as agents are pre-declared before place class
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
        index=3;//start at home
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
        currentPlace=schedule->getNextLocation();
        //expensive - only needed if agents need direct agnt-to-agent interactions in a place -
        //moveTo(currentPlace);
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
}
void agent::initTravelSchedule(){       
   schedule=new  travelSchedule();   
}
void agent::cough()
{
        //breathInto(place) - scale linearly with the time spent there - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        if (diseased) places[currentPlace]->increaseContamination(disease::shedInfection()*schedule->getTimeAtCurrentPlaceInHours()/24.);
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief The model contains all the agents and places, and steps them through time
*/
class model{
    std::vector<agent*> agents;
    std::vector<place*> places;
    int nAgents=6000000;
    std::ofstream output;
public:
    model(){
        randomizer r=randomizer::getInstance();
        r.setSeed(1);
        //output file
        output.open("diseaseSummary.csv");
        //header line
        output<<"step,susceptible,infected,recovered"<<std::endl;
        //Initialisation can be slow - check the timing
        auto start=timeReporter::getTime();
        init();
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Initialisation took: ", start,end);
        travelSchedule x;
    }
    void init(){
        //create homes
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
        //create work places
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
        //create buses
        for (int i=nAgents/3+nAgents/10;i<nAgents/3+nAgents/10+nAgents/30;i++){
            place* p=new place();
            places.push_back(p);
            places[i]->ID=i;
        }
        //allocate 30 agents per bus
        for (int i=0;i<agents.size();i++){
            assert(places[i/30+nAgents/3+nAgents/10]!=0);
            agents[i]->setTransport(places[i/30+nAgents/3+nAgents/10]);
        }
        for (int i=0;i<agents.size();i++){
            agents[i]->initTravelSchedule();
        }
        std::cout<<"Built "<<agents.size()<<" agents and "<<places.size()<<" places."<<std::endl;
        //set off the disease!
        agents[0]->diseased=true;
    }

    void step(int num){

        //update the places - changes contamination level

        for (int i=0;i<places.size();i++){
            places[i]->update();
        }
        //counts the totals
        int infected=0,recovered=0;
        //do disease - synchronous update (i.e. all agents contaminate before getting infected) so that no agent gets to infect ahead of others.
        //alternatively could be randomized...depends on the idea of how a location works...places could be sub-divided to mimic spatial extent for example.
        
        for (int i=0;i<agents.size();i++){
            agents[i]->cough();
        }
        //the disease progresses
        
        for (int i=0;i<agents.size();i++){
            agents[i]->disease();
        }
        //accumulate totals
        for (int i=0;i<agents.size();i++){
            if (agents[i]->diseased)infected++;
            if (agents[i]->immune)recovered++;
        }
        //move around, do other things in a location
        
        for (int i=0;i<agents.size();i++){
            agents[i]->update();
        }

        //output a summary .csv file
        output<<num<<","<<agents.size()-infected-recovered<<","<<infected<<","<<recovered<<std::endl;
        //show the step number every 10 steps

        if (num%10==0)std::cout<<"Step "<<num<<std::endl;
        
        for (int i=0;i<places.size();i++){
            //places[i]->show();
        }
    }
    
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
int main(int argc, char **argv) {
    
    std::cout<<"Model version -0.9"<<std::endl;
    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run started at: "<<ctime(&t)<<std::endl;

    model m;
    int nSteps=10;
    //start a timer to record the execution time
    auto start=timeReporter::getTime();
    //loop over time steps
    for (int step=0;step<nSteps;step++){
        m.step(step);
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
 * For this reason at the moment all code is in a single file. this can become unwieldy in a larger application though. \n
 * 
 * The current objective is to be able to model a simple disease, and to tie this to agent behaviour at the scale of an entire country.
 * @subsection Main Main ideas
 * Agents move between places according to a given fixed travel schedule. Places include transport vehicles. \n
 * In each place, agents with a disease can add contamination, which then decays exponentially over time. \n
 * The release of total contamination depends linearly on time spent in each location. \n
 * In a contaminated location, susceptible agents can pick up the infection - they then recover with a fixed chance per timestep, and are subsequently immune.
 * @subsection Compiling Compiling the model
 * On a linux system with g++ installed just do \n
 * g++ -o agentModel -O3 main.cpp
 * @subsection Run Running the model
 * At present this is a simple command-line application - just type the executable name (agentModel above) and then return.
 **/

