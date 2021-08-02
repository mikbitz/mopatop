#include <iostream>
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
    void show(bool);
};
class agent{
public:
    int ID;
    place* home;
    place* work;
    //transport vehicles are places, albeit moveable!
    place* bus;
    agent(){
        home=0;
        work=0;
        bus=0;
    }
    void doStuff(){
        atHome();
        enterTransport(home,bus);//load people into busstop rather than direct into bus?
        inTransit();//trip chaining here?
        leaveTransport(bus,work);
        atWork();
        enterTransport(work,bus);
        inTransit();
        leaveTransport(bus,home);
        atHome();
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
int main(int argc, char **argv) {
    std::vector<agent*> agents;
    std::vector<place*> places;
    int nAgents=60000;
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
        assert(places[i/10+200]!=0);
        agents[i]->setWork(places[i/10+200]);
    }
    //create buses
    for (int i=nAgents/3+nAgents/10;i<nAgents/3+nAgents/10+nAgents/30;i++){
        place* p=new place();
        places.push_back(p);
        places[i]->ID=i;
    }
    //allocate 30 agents per bus
    for (int i=0;i<agents.size();i++){
        assert(places[i/30+260]!=0);
        agents[i]->setTransport(places[i/30+260]);
    }
    //move between the two
    for (int step=0;step<10;step++){
        for (int i=0;i<agents.size();i++){
            agents[i]->doStuff();
        }
    }
    for (int i=0;i<places.size();i++){
        places[i]->show();
    }
    return 0;
}
