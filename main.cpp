#include <iostream>
#include<vector>
#include<set>
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
        goHome();
        goToWork();
        goHome();
    }
    void goHome(){
        work->remove(this);
        home->add(this);
    }
    void goToWork(){
        home->remove(this);
        work->add(this);
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
    for (int i=0;i<2;i++){
        place* p=new place();
        places.push_back(p);
        places[i]->ID=i;
    }
    for (int i=0;i<60000;i++){
        agent* a=new agent();
        agents.push_back(a);
        agents[i]->ID=i;
        agents[i]->setHome(places[0]);
        agents[i]->setWork(places[1]);
    }
    for (int step=0;step<1;step++){
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
