#ifndef MODELFACTORY_H
#define MODELFACTORY_H
#include "agent.h"
#include "places.h"
#include "remoteTravel.h"
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
 * @brief File containing the definition of the factories for making models
 * 
 * @author Mike Bithell
 * @date 30/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief A class to store a list of possible travel destinations 
    @details This is a class with a static list of places that can be visited as specified in \ref remoteTravel.h \n
    Since the list is static there is only one copy where this list is intialised (in \ref modelFactory at present)\n
    Agents then access travelLocations via their name.
   */
class travelList{
public:
    /** @brief a keyed list of travel locations using their \b unique name */
    static std::map<std::string,remoteTravel*> travelLocations;
    /** @brief static function to add named locations to the list 
        @param name the unique name of the location
        @param parameters the model parmeter settings - needs to be passed to the \ref remoteTravel object
        @param otherDomain a flag to denote whether this location is actually located in another MPI domain*/
    static void add(std::string name,parameterSettings& parameters,std::vector<place*>& places,bool otherDomain=false){
        travelLocations[name]=new remoteTravel(parameters,places,otherDomain);
    }
};
/** @brief The modelFactory itself is a (virtual) base class
    @details The various model factories are sub-classed from this class below.\n
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
    virtual void createAgents(parameterSettings& parameters,std::vector<agent*>& agents,std::vector<place*>& places,std::string domain)=0;
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
    void createAgents(parameterSettings& parameters,std::vector<agent*>& agents,std::vector<place*>& places,std::string domain){

        std::cout<<"Starting simple one place generator..."<<std::endl;
        std::cout<<"Creating places ...";
        //create homes - just a single location for everyone in this case!
        place* p=new place(parameters);
        places.push_back(p);
        places[0]->setID(0);
        std::cout<<std::endl;
        std::cout<<"Creating agents ...";
        long k=0;
        //fraction indicates when each extra 10% of agents have been created
        long fr=parameters.get<long>("run.nAgents")/10;
        //check for tiny numbers of agents
        if (fr==0)fr=parameters.get<long>("run.nAgents");
        //allocate all agents the same home - no travel in this case
        agents.resize(parameters.get<long>("run.nAgents"));
        #pragma omp parallel for
        for (long i=0;i<parameters.get<long>("run.nAgents");i++){
            agent* a=new agent();
            a->setHome(places[0]);
            //some schedules assume that work and tranport exist - set these so as not to cause a model crash
            a->setTransport(places[0]);
            a->setWork(places[0]);
            //agent internal thread ID counter not thread safe, so set explicitly
            a->setID(i);
            k++;
            if (k%fr==0)std::cout<<k<<"...";
            agents[i]=a;
        }
        std::cout<<std::endl;

        //set up travel schedule - same for every agent at the moment -  at home at exactly the same times
        for (long i=0;i<agents.size();i++){
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
    void createAgents(parameterSettings& parameters, std::vector<agent*>& agents,std::vector<place*>& places,std::string domain){

        long nAgents=parameters.get<long>("run.nAgents");
        //default values to allocate rough numbers of agents to types of place
        int agentsPerHome=3;
        int agentsPerWorkPlace=10;
        int agentsPerBus=30;
        //excess allows for numbers of agents not strictly divisible by agentsPerHome - create at most one extra place.
        int excess=((nAgents % agentsPerHome)>0);//if remainder of nAgents on division by agentsPerHome is greater than zero, this will result in 1, otherwise zero
        long nHomes=nAgents/agentsPerHome+excess;
        //now get number of workplaces
        excess=((nAgents % agentsPerWorkPlace)>0);//as with homes, allow for not divisible by 10
        int nWork=nAgents/agentsPerWorkPlace+excess;
        //and number of transport vehciles, assumed buses
        excess=((nAgents % agentsPerBus)>0);//allow for not exactly 30 agents per bus
        long nBus=nAgents/agentsPerBus+excess;
        //check for very small numbers of agents
        if (nHomes==0) nHomes=1;
        if (nWork==0) nWork=1;
        if (nBus==0) nBus=1;
        //faster to resize the array here, then create places in a parallel loop (individual place memory allocations get done in parallel, but loop is thread-safe)
        //Currently 4.5e9 agents + 2.1 e9 places on 64 threads on HPC takes about 45 minutes.
        //one could also subdivide this resizing and gain something in speed by making indiviual sub-vectors on each thread in an omp parallel loop then concatenating them in an omp critical section?
        //something like this might gain a little bit: - but only seems to save about a minute on the above time...
        //#pragma omp parallel
        //{
        //    long z=(nHomes+nWork+nBus)/omp_get_num_threads();//allocate part of the vector on each thread
        //    //this variable is private to the thread
        //    std::vector<place*> vec_private(z);
        //    //add to the global vector, but avoid races.
        //    #pragma omp critical
        //    places.insert(places.end(), vec_private.begin(), vec_private.end());
        //}
        //even so after the above, need to make sure we really are the right size in case the number of threads doesn't exactly divide the the size of the vector

        places.resize(nHomes+nWork+nBus);
        
        std::cout<<"Starting simple mobile generator..."<<std::endl;
        std::cout<<"Creating homes ..."<<std::endl;

        #pragma omp parallel for
        for (long i=0;i<nHomes;i++){
            place* p=new place(parameters);
            p->setID(i);
            places[i]=p;
        }
        
        std::cout<<"Creating agents ...";
        long k=0;
        //fraction indicates when each extra 10% of agents have been created
        long fr=nAgents/10;
        //check for tiny numbers of agents
        if (fr==0)fr=nAgents;
        //allocate agentsPerHome agents per home - as far as possible - any excess over nAgents/agentsPerHome go into the excess Home as defined above (either one or two if agentsPerHome==3 for example)
        agents.resize(nAgents);
        #pragma omp parallel for
        for (long i=0;i<nAgents;i++){
            agent* a=new agent();
            //set the agent ID explicitly - internal agent ID counter is not thread safe 
            a->setID(i);
            assert(places[i/agentsPerHome]!=0);
            a->setHome(places[i/agentsPerHome]);
            k++;
            if (k%fr==0)std::cout<<k<<"...";         
            agents[i]=a;
        }
        std::cout<<std::endl;
        //create work places - (nAgents / agentsPerWorkPlace)  as many as agents - add them on to the end of the place list.
        std::cout<<"Creating workplaces ..."<<std::endl;

        #pragma omp parallel for
        for (long i=nHomes;i<nHomes+nWork;i++){
            place* p=new place(parameters);
            p->setID(i);          
            places[i]=p;
        }
        //shuffle agents so household members get different workplaces - can this be parallelised?
        random_shuffle(agents.begin(),agents.end());
        //allocate agentsPerWorkPlace agents per workplace
        #pragma omp parallel for        
        for (long i=0;i<agents.size();i++){
            assert(places[i/agentsPerWorkPlace+nHomes]!=0);
            agents[i]->setWork(places[i/agentsPerWorkPlace+nHomes]);
        }
        std::cout<<"Creating transport ..."<<std::endl;
        //create buses - (nAgents / agentsPerBus) since agentsPerBus agents per bus. add them to the end of the place list again

        #pragma omp parallel for  
        for (long i=nHomes+nWork;i<nHomes+nWork+nBus;i++){
            place* p=new place(parameters);
            p->setID(i);
            places[i]=p;
        }
        //allocate agentsPerBus agents per bus - since agents aren't shuffled again, those in similar workplaces will tend to share buses. 
        #pragma omp parallel for
        for (long i=0;i<agents.size();i++){
            assert(places[i/agentsPerBus+nHomes+nWork]!=0);
            agents[i]->setTransport(places[i/agentsPerBus+nHomes+nWork]);
        }
        //set up travel schedule - same for every agent at the moment - so agents are all on the bus, at work or at home at exactly the same times
        #pragma omp parallel for
        for (long i=0;i<agents.size();i++){
            agents[i]->initTravelSchedule(parameters);
        }
        //report intialization to std out 
        std::cout<<"Built "<<agents.size()<<" agents and "<<places.size()<<" places."<<std::endl;
        //create some remote places to travel to - local ones are on this MPI domain, remote another one.
        //local only to domain b - for domain other than b it will be availabe here but labelled as remote 
        travelList::add("NewYork",parameters,places,domain=="b");
        //local only to domain a
        travelList::add("London",parameters,places,domain=="a");

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
