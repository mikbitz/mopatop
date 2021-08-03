#include <iostream>
#include<functional>
#include<vector>
#include<set>
#include<assert.h>
class agent;
class place{
public:
    int ID;
    //unique list of current people in this place
    std::set<agent*> occupants;
    place(){ID=0;}
    void add(agent* a){
        occupants.insert(a);
    }
    void remove(agent* a){
        occupants.erase(a);
    }
    void update(){}
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
    bool update(){return false;}
    //auto n=[&](agent* a){a->atHome();};
    //void operator()(agent* a){a->atHome();}
};
class agent{
public:
    int ID;
    place* currentPlace;
    place* home;
    place* work;
    //transport vehicles are places, albeit moveable!
    place* bus;
    
    std::vector<placeChanger*> p;
    int schedule;
    agent(){
        home=0;
        work=0;
        bus=0;
    }
    void initTravelSchedule(){
        p.push_back(new placeChanger(this,home,bus));//load people into busstop (transportHub) rather than direct into bus - here they wait
        p.push_back(new placeChanger(this,bus,work));
        p.push_back(new placeChanger(this,work,bus));
        p.push_back(new placeChanger(this,bus,home));        
        schedule=0;
    }
    void update(){
        //Use the base travel schedule - initialised at home for everyone

        //auto k=[&](){atHome();};
        //k();
        if (ID==0)std::cout<<schedule<<std::endl;
        p[schedule]->update();
        schedule++;
        schedule=schedule % p.size();

        //enterTransportHub();
        //enterTransport(home,bus);
        //inTransit();//trip chaining here? how to handle trips across multiple transport hubs? how to do schools (do parents load up childer?) and shops?
        //leaveTransport(bus,work);
        //atWork();//this could involve travelling too - e.g. if delivery driver 
        //enterTransport(work,bus);
        //inTransit();
        //leaveTransport(bus,home);
        //exitTransportHub(); go from busstop (or station etc.) to home
        //atHome();//people might be at some other location overnight - e.g. holiday or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
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
        std::cout<<"at Home"<<std::endl;
    }
    void atWork(){
    }
    void inTransit(){
    }
    void setHome(place* p){
        home=p;
        //start all agents at home
        home->add(this);
    }
    void setWork(place* p){
        work=p;
    }
    void setTransport(place* p){
        bus=p;
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

//    template <typename F>
//void ask(std::set<agent*> ps,F f){
//    for (auto& p: ps) f(*p);
//}
    template <typename F>
void ask(agent* ps,F f){
    f(ps);
}
int main(int argc, char **argv) {
//agent* w=new agent();
//auto n=[&](agent* a){a->atHome();};
//ask(w,n);
    std::vector<agent*> agents;
    std::vector<place*> places;
    int nAgents=60000;
    int nSteps=10;
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
    //move between the places
    for (int step=0;step<nSteps;step++){
        for (int i=0;i<agents.size();i++){
            agents[i]->update();
        }
        for (int i=0;i<places.size();i++){
            places[i]->update();
        }
    }
    for (int i=0;i<places.size();i++){
        //places[i]->show();
    }
    return 0;
}
