#include <iostream>
#include<algorithm>
#include <random>
#include<fstream>
#include<vector>
#include<chrono>
#include<set>
#include<string>
#include<assert.h>
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//simple static class to abstract the clunky C++ chrono system
class timeReporter{
public:
    //get the time now
    static std::chrono::time_point<std::chrono::steady_clock> getTime(){return std::chrono::steady_clock::now();}
    //show the interval between two time points in milliseconds
    static void showInterval(std::string s,std::chrono::time_point<std::chrono::steady_clock> start,std::chrono::time_point<std::chrono::steady_clock> end){
        std::cout<<s<<float(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count())/1000<<" seconds"<<std::endl;
    }

};
//------------------------------------------------------------------------
//Set up a wrapper class that will provide uniform random numbers between 0 and 1
//Use a singleton so there is only one random seqeunce across all agents
class randomizer {
public:
    static randomizer& getInstance(){ 
        if (instance==NULL){
            instance=new randomizer();
        }
        return *instance;
    }
    std::uniform_real_distribution<> uniform_dist;
    // Use mersenne twister with fixed seed as the random number engine
    std::mt19937 twister;
public:
    //~randomizer(){
    //  clean();
   // }
    double number(){
     return uniform_dist(twister);
    }
    void setSeed(int s){
        std::cout<<"randomizer seed set to "<<s <<std::endl;
        twister.seed(s);
    }
private:
    
    static randomizer* instance;
    randomizer(){
        uniform_dist=std::uniform_real_distribution<> (0,1);
        std::cout<<"A randomizer was set up with seed 0" <<std::endl;
        twister.seed(0);
    }
    
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
class place{
public:
    int ID;
    float contaminationLevel,fractionalDecrement;
    //unique list of current people in this place
    //currently this is not used...seems to add about 20% to memory requirement
    std::set<agent*> occupants;

    place(){ID=0;contaminationLevel=0.;fractionalDecrement=0.1;}
    void add(agent* a){
        occupants.insert(a);
    }
    void remove(agent* a){
        occupants.erase(a);
    }
    void increaseContamination(float amount){contaminationLevel+=amount;}
    float getContaminationLevel(){return contaminationLevel;}
    //contamination decays exponentially
    void update(){contaminationLevel*=fractionalDecrement;}
    void show(bool);
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//move agents between locations
class placeChanger{
    place *origin,*destination;
public:
    placeChanger(){origin=nullptr;destination=nullptr;}
    placeChanger(agent* a, place* o,place* d):origin(o),destination(d){ 
        origin->remove(a);
        destination->add(a);
    }//place changing shouldn't really happen in the constructor! these lines are only needed if agents will interact directly at the place, in any case 
    place* update(){return destination;}
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class agent{
public:
    int ID;
    place* currentPlace;
    place* home;
    place* work;
    //transport vehicles are places, albeit moveable!
    place* vehicle;
    //holds the travel schedule
    std::vector<placeChanger*> travel;
    //where we have got to through the schedule
    int schedule;
    //disease parameters
    bool diseased,immune;
    agent(){
        home=0;
        work=0;
        vehicle=0;
        diseased=false;
    }
    void initTravelSchedule(){
        travel.push_back(new placeChanger(this,home,vehicle));//load people into busstop (or transportHub) rather than direct into bus? - here they would wait
        travel.push_back(new placeChanger(this,vehicle,work));//trip chaining? how to handle trips across multiple transport hubs? how to do schools (do parents load up childer?) and shops?
        travel.push_back(new placeChanger(this,work,vehicle));
        travel.push_back(new placeChanger(this,vehicle,home));        
        schedule=0;
    }
    void update(){
        //Use the base travel schedule - initialised at home for everyone
        currentPlace=travel[schedule]->update();
        schedule++;
        schedule=schedule % travel.size();
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
    }
    void cough(){
        //breathInto(place) - masks could go here (what about surfaces? -second contamination factor?)
        if (diseased) currentPlace->increaseContamination(0.01);
    }
    void disease(){
        //very very simple disease...
        //recovery
        if (diseased){
            if (0.002>randomizer::getInstance().number()) {diseased=false ; immune=true;}
        }
        //infection
        if (currentPlace->getContaminationLevel()>randomizer::getInstance().number() && !immune)diseased=true;
        //immunity loss could go here...
    }
    void atHome(){
        if (ID==0)std::cout<<"at Home"<<std::endl;
        if (ID==1)std::cout<<"cough! at place ID "<<currentPlace->ID<<": "<<home->getContaminationLevel()<<std::endl;
    }
    void atWork(){
        if (ID==0)std::cout<<"at Work"<<std::endl;
    }
    void inTransit(){
        if (ID==0)std::cout<<"on Bus"<<std::endl;
    }
    void setHome(place* p){
        home=p;
        //start all agents at home - if using the occupants list, add to the home place
        //home->add(this);
        currentPlace=home;
    }
    void setWork(place* p){
        work=p;
    }
    void setTransport(place* p){
        vehicle=p;
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
class model{
    std::vector<agent*> agents;
    std::vector<place*> places;
    int nAgents=600;
    std::ofstream output;
public:
    model(){
        randomizer r=randomizer::getInstance();
        r.setSeed(1);
        //output file
        output.open("diseaseSummary");
        //header line
        output<<"step,susceptible,infected,recovered"<<std::endl;
        //Initialisation can be slow - check the timing
        auto start=timeReporter::getTime();
        init();
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Initialisation took: ", start,end);
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
        int infected=0,recovered=0;
        //do disease - synchronous update (i.e. all agents contaminate before getting infected) so that no agent gets to infect ahead of others.
        //alternatively could be randomized...depends on the idea of how a location works...places could be sub-divided to mimic spatial extent for example.
        for (int i=0;i<agents.size();i++){
            agents[i]->cough();
        }
        //the disease progresses
        for (int i=0;i<agents.size();i++){
            agents[i]->disease();
            if (agents[i]->diseased)infected++;
            if (agents[i]->immune)recovered++;
        }
        //move around, do other things in a location
        for (int i=0;i<agents.size();i++){
            agents[i]->update();
        }
        output<<num<<","<<agents.size()-infected-recovered<<","<<infected<<","<<recovered<<std::endl;
        std::cout<<"Infected "<<infected<<std::endl;
        
        for (int i=0;i<places.size();i++){
            //places[i]->show();
        }
    }
    
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
int main(int argc, char **argv) {
    model m;
    int nSteps=1000;
    auto start=timeReporter::getTime();
    for (int step=0;step<nSteps;step++){
        m.step(step);
    }
    auto end=timeReporter::getTime();
    timeReporter::showInterval("Execution time after initialisation: ",start,end);
    return 0;
}

