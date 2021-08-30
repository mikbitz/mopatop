#ifndef MODELFACTORY_H
#define MODELFACTORY_H
#include "agent.h"
#include "places.h"
/* A program to model agents moving between places
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @file modelFactory.h 
 * @brief File containing the definition of the agent class
 * 
 * @author Mike Bithell
 * @date 30/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief These classes allow for the creation of agent populations and placesusing a variety of different methods 
    @details The modelFactory itself is a (virtual) base class - the various factories are sub-classed from this class below.\n
    These classes are expected to be called directly from a model object.
    A \ref modelFactorySelector selector then allows for a given factory to be chosen by name using a string
*/
class modelFactory{
public:
    modelFactory(){;}
    virtual ~modelFactory(){;}
    /** @brief virtual method defining the signature of the createAgents method 
       @details This method cannot be called from this class - rather a sub-class must overload this method, which then\n
      has to be accessed by creating a pointer to the sub-class.
      @param parameters A reference to the model parameterSettings object
      @param agents A reference to the model object's list of agents
      @param places* A reference to the model object's list of places
        */
    virtual void createAgents(parameterSettings& parameters,std::vector<agent*>& agents,std::vector<place*>& places)=0;
};
/** @brief Create a set of agents that all know only about one place, and remain there for all time, irespective of travel schedule\n 
    @details First the place is created, then agents, who all set this one place as home, work and transport. The latter two are set\n
    in case at some point a different travel schedule is picked, in which case attemtps to move will all still just end up in the home location\n
     \ref modelFactorySelector knows this as "simpleOnePlace" . Use this class by creating a pointer to the sub-class:-
     \code
     modelFactory* F=new simpleOnePlaceFactory();
     \endcode
     See \ref modelFactorySelector
*/
class simpleOnePlaceFactory:public modelFactory{
    /** @brief method to overlaod the createAgents method in the base class
       @details This method has to be accessed by creating a pointer to this sub-class.
      @param parameters A reference to the model parameterSettings object
      @param agents A reference to the model object's list of agents
      @param places* A reference to the model object's list of places*/
    void createAgents(parameterSettings& parameters,std::vector<agent*>& agents,std::vector<place*>& places){

        std::cout<<"Starting simple one place generator..."<<std::endl;
        std::cout<<"Creating places ...";
        //create homes - just a single location for everyone in this case!
        place* p=new place(parameters);
        places.push_back(p);
        places[0]->setID(0);
        std::cout<<std::endl;
        std::cout<<"Creating agents ...";
        int k=0;
        //allocate all agents the same home - no travel in this case
        for (int i=0;i<parameters.get<long>("run.nAgents");i++){
            agent* a=new agent();
            agents.push_back(a);
            agents[i]->ID=i;
            agents[i]->setHome(places[0]);
            //some schedules assume that work and tranport exist - set these so as not to cause a model crash
            agents[i]->setTransport(places[0]);
            agents[i]->setWork(places[0]);
            k++;
            if (k%100000==0)std::cout<<k<<"...";
        }
        std::cout<<std::endl;

        //set up travel schedule - same for every agent at the moment -  at home at exactly the same times
        for (int i=0;i<agents.size();i++){
            agents[i]->initTravelSchedule(parameters);
        }
        //report intialization to std out 
        std::cout<<"Built "<<agents.size()<<" agents and "<<places.size()<<" places."<<std::endl;

    }
};
/** @brief Create a set of agents that all have three places they know about, home, work and transport\n 
    @details First the home place is created, then agents, who all set this one place as home. Work places and transport vary by agent \n
    Currently there are 3 agenst per home, 10 agents per work place, and 30 per tranport (a bus!)\n
    If the travel schedule is set to stationary, however, only the home place will get used.\n
     \ref modelFactorySelector knows this as "simpleMobile".Use this class by creating a pointer to the sub-class:-
     \code
     modelFactory* F=new simpleMobileFactory();
     \endcode
    See \ref modelFactorySelector
*/
class simpleMobileFactory:public modelFactory{
        /** @brief method to overlaod the createAgents method in the base class
       @details This method has to be accessed by creating a pointer to this sub-class.
      @param parameters A reference to the model parameterSettings object
      @param agents A reference to the model object's list of agents
      @param places* A reference to the model object's list of places*/
    void createAgents(parameterSettings& parameters, std::vector<agent*>& agents,std::vector<place*>& places){

        long nAgents=parameters.get<long>("run.nAgents");
        
        std::cout<<"Starting simple mobile generator..."<<std::endl;
        std::cout<<"Creating homes ..."<<std::endl;
        for (int i=0;i<nAgents/3;i++){
            place* p=new place(parameters);
            places.push_back(p);
            places[i]->setID(i);
        }
        std::cout<<"Creating agents ...";
        int k=0;
        //allocate 3 agents per home
        for (int i=0;i<nAgents;i++){
            agent* a=new agent();
            agents.push_back(a);
            agents[i]->ID=i;
            agents[i]->setHome(places[i/3]);
            k++;
            if (k%100000==0)std::cout<<k<<"...";
        }
        std::cout<<std::endl;
        //create work places - one tenth as many as agents - add them on to the end of the place list.
        std::cout<<"Creating workplaces ..."<<std::endl;
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
        std::cout<<"Creating transport ..."<<std::endl;
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
            agents[i]->initTravelSchedule(parameters);
        }
        //report intialization to std out 
        std::cout<<"Built "<<agents.size()<<" agents and "<<places.size()<<" places."<<std::endl;

    }
};
/** @brief A class to pick one of a number of possible agent factories 
    @details This is a static class used to define a pointer to a \ref modelFactory \n
    Each model factory can be selected using a name passed into the \ref select method using\n
    a string. This is used in the model class along with the \ref parameterSettings to choose\n
    what the model initialization of places and agents will look like.
   */
class modelFactorySelector{
public:
    /** @brief choose the model factory
     * @param name A string that names one  \ref modelFactory
     * @return A reference to the requested  \ref modelFactory
     * Example use:-
     \code
        modelFactory& F=modelFactorySelector::select(parameters("model.type"));
        //create the distribution of agents, places and transport
        F.createAgents(parameters,agents,places);
     \endcode
     */
    static modelFactory&  select(std::string name){
        modelFactory* F=nullptr;
        if (name=="simpleOnePlace")F=new simpleOnePlaceFactory();
        if (name=="simpleMobile")  F=new simpleMobileFactory();
        if (F==nullptr)std::cout<<"Name "<<name<<" not recognised in modelFactorySelector"<<std::endl;
        assert(F!=nullptr);
        return *F;
    }
};
#endif
