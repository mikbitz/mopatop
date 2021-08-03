#include <iostream>
#include<functional>
#include<vector>
#include<set>
#include<assert.h>
class agent;
class place{
public:
    int ID;
    float contaminationLevel,decrement;
    //unique list of current people in this place
    std::set<agent*> occupants;
    place(){ID=0;contaminationLevel=0.;decrement=0.9;}
    void add(agent* a){
        occupants.insert(a);
    }
    void remove(agent* a){
        occupants.erase(a);
    }
    void increaseContamination(float amount){contaminationLevel+=amount;}
    float getContaminationLevel(){return contaminationLevel;}
    void update(){contaminationLevel*=decrement;}
    void show(bool);
};
class placeChanger{
    place* origin,*destination;
public:
    placeChanger(){origin=nullptr;destination=nullptr;}
    placeChanger(agent* a, place* o,place* d):origin(o),destination(d){ 
        origin->remove(a);
        destination->add(a);
    }
    place* update(){return destination;}
};
class agent{
public:
    int ID;
    place* currentPlace;
    place* home;
    place* work;
    //transport vehicles are places, albeit moveable!
    place* vehicle;
    
    std::vector<placeChanger*> travel;
    int schedule;
    bool diseased;
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
        if (ID==0)std::cout<<schedule<<std::endl;
        currentPlace=travel[schedule]->update();
        schedule++;
        schedule=schedule % travel.size();
        if (diseased)currentPlace->increaseContamination(0.1);
        if (currentPlace->getContaminationLevel()>0.4)diseased=true;
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
    }
    void enterTransport(place* origin,place* transportMode){
        origin->remove(this);
        transportMode->add(this);
    }
    void leaveTransport(place* transportMode,place* destination){
        transportMode->remove(this);
        destination->add(this);
    }
    void atHome(){
        //stigmergic disease at all locations? - how to update places consistently (and simply) with agents?
        //breathInto(place)? (what about surfaces?)
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
        //start all agents at home
        home->add(this);
        currentPlace=home;
    }
    void setWork(place* p){
        work=p;
    }
    void setTransport(place* p){
        vehicle=p;
    }

};
void place::show(bool listAll=false){
    std::cout<<"Place ID "<<ID<<" has "<<occupants.size()<<" occupants"<<std::endl;
    if (listAll){
        std::cout<<"List of Occupant IDs:- "<<std::endl;
        for (auto o : occupants){
            std::cout <<o->ID<<std::endl;
        }
    }
}


int main(int argc, char **argv) {

    std::vector<agent*> agents;
    std::vector<place*> places;
    int nAgents=60;
    int nSteps=1000;
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
    agents[0]->diseased=true;
    //move between the places
    for (int step=0;step<nSteps;step++){
        int ill=0;
        for (int i=0;i<agents.size();i++){
            agents[i]->update();
            if (agents[i]->diseased)ill++;
        }
        for (int i=0;i<places.size();i++){
            places[i]->update();
        }
        std::cout<<"Infected "<<ill<<std::endl;
    }
    for (int i=0;i<places.size();i++){
        //places[i]->show();
    }
    return 0;
}
//reminders of how to do some function cally things
        //auto k=[&](){atHome();};
        //k();
//agent* w=new agent();
//auto n=[&](agent* a){a->atHome();};
//    template <typename F>
//void ask(std::set<agent*> ps,F f){
//    for (auto& p: ps) f(*p);
//}
//    template <typename F>
//void ask(agent* ps,F f){
//    f(ps);
//}
//ask(w,n);
    //auto n=[&](agent* a){a->atHome();};
    //void operator()(agent* a){a->atHome();}
