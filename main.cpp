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
    agent(){
        home=0;
        work=0;
    }
    void doStuff(){
        atHome();
        transport(home,work);
        atWork();
        transport(work,home);
        atHome();
    }
    void transport(place* origin,place* destination){
        origin->remove(this);
        destination->add(this);
    }
    void atHome(){
        //work->remove(this);
        //home->add(this);
    }
    void atWork(){
        //home->remove(this);
        //work->add(this);
    }
    void setHome(place* p){
        home=p;
    }
    void setWork(place* p){
        work=p;
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
    //create homes
    for (int i=0;i<200;i++){
        place* p=new place();
        places.push_back(p);
        places[i]->ID=i;
    }
    //allocate 3 agents per home
    for (int i=0;i<600;i++){
        agent* a=new agent();
        agents.push_back(a);
        agents[i]->ID=i;
        agents[i]->setHome(places[i/3]);
    }
    //create work places
    for (int i=200;i<260;i++){
        place* p=new place();
        places.push_back(p);
        places[i]->ID=i;
    }
    //allocate 10 agents per workplace
    for (int i=0;i<600;i++){
        assert(places[i/10+200]!=0);
        agents[i]->setWork(places[i/10+200]);
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
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
