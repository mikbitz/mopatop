#include"agent.h"
#include"places.h"
#include "remoteTravel.h"
#include "modelFactory.h"
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
 * @file agent.cpp 
 * @brief File containing methods that require definitions of other classes to be available
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#include"agent.h"
#include"places.h"
#include "remoteTravel.h"
#include "modelFactory.h"

//static list of travel destinations away from home
//std::map<std::string,remoteTravel*> travelList::travelLocations;
std::multimap<unsigned short,airport*> airportList::airports;
//------------------------------------------------------------------------
agent::agent(){

    _diseased=false;
    _immune=false;
    _recovered=false;
    _alive=true;
    _active=true;
    _leaver=false;
    _locationIsRemote=false;
    _mobile=true;
    //this is supposed to set a unique ID, but *NOT* threadsafe!! Set the ID instead at agent creation.
    ID=nextID;
    nextID++;
}
//------------------------------------------------------------------------
void agent::moveTo(placeTypes location){
        assert(places[location]!=nullptr);
        places[currentPlace]->remove(this);
        places[location]->add(this);
        currentPlace=location;
}
//------------------------------------------------------------------------
void agent::process_disease(randomizer& r){
        //recovery
        if (diseased()){
            if (disease::die(r))                {die();}
            if (alive() && disease::recover(r)) {recover();}
        }
        //infection
        assert(places[currentPlace]!=nullptr);
        if (alive() && !immune() && disease::infect(places[currentPlace]->getContaminationLevel(),r) )becomeInfected();
        //immunity loss could go here...
}
//------------------------------------------------------------------------
void agent::atHome(){
    //if (ID==0)std::cout<<"at Home "<<timeStep::getTimeOfDay()<<std::endl;
    int T=timeStep::getTimeOfDay();
    int day=timeStep::getDayOfWeek();
    //if(ID==0)std::cout<<(scheduleType==stationary)<<std::endl;
    if (_mobile && T>=800 && T<900 && day < 5)currentPlace=vehicle;//go to work if mobile and unless the weekend (days 5/6)
}
//------------------------------------------------------------------------


void agent::atWork(){
    //if (ID==0)std::cout<<"at Work"<<timeStep::getTimeOfDay()<<std::endl;
    if (timeStep::getTimeOfDay()>=1700)currentPlace=vehicle;
}
//------------------------------------------------------------------------

void agent::inTransit(){
    //if (ID==0)std::cout<<"travelling"<<timeStep::getTimeOfDay()<<std::endl;
    int T=timeStep::getTimeOfDay();
    if (T>=1800)
        currentPlace=home;
    else 
       if (T>=900 && T<1700) currentPlace=work;
    
}
//------------------------------------------------------------------------
void agent::update()
{       
        //rules to move agents between places
        //different rule sets may apply depending on flags for switching between sets
        //within a set at the moment the rules are typically subsumption style 
        //i.e. the agents run all rules in fixed order - with later rules possibly overriding earlier onees
        //this means rules must be carefully set to make sure this works as intended

        if (verySick()) goToHospital();
        if (currentPlace==hospital){
            inHospital();
        }
        if (needFood()) goShopping();
        if (currentPlace==shop){
            inShop();
        }
        //what we did on our holidays
        if (holidayTime()){
         goOnHoliday();
         returnFromHoliday();
         arriveHome();
        }else{
         if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
         if (currentPlace==vehicle)inTransit();//trips to and from work only
         if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver
        }
        //moving agents between data structures is expensive - only needed if agents need direct agent-to-agent interactions in a place -
        //might be made cheaper by allowing agents to be present in multiple places, but only active in one.
        //(this could allow for remote meetings/phone calls?!)
        //moveTo(currentPlace);

}
bool agent::holidayTime()
{
    //holiday on 1st of June at midnight!
    //if (timeStep::getMonth()==5 && timeStep::getDayOfMonth()==0) return true;
    return false;
}

//------------------------------------------------------------------------
void agent::goOnHoliday(){
    //need flag here to test if already on holiday
    //find a local airport (replace 0 here with my country code)
    int countryCode=0;
    auto air=airportList::getAirports(countryCode);
    //select one airport (at random?)
    int rnd=0;
    placeCache[0]=air[rnd]->getPlace();
    //find a remote airport from code (at random?)- assumed there that there is a route from country 0 to country 1
    int remoteCountry=1;
    auto farfarAway=air[rnd]->getDepartures(remoteCountry);
    //get the plane
    int rnd2=0;
    placeCache[1]=farfarAway[rnd2]->getPlane();
    //and the remote airport
    placeCache[2]=farfarAway[rnd2]->getDestination()->getPlace();
    
    
    //find transport to the airport - get on when near plane schedule
    //go to airport and wait until plane ready
    //get on (a specific timetabled) plane - one plane a day between destinations? airport needs timetable, planes attached to a departure time and have a duration 
    //get off plane at remote airport
    //travel locally to hotel
    //stay in hotel until leave time
    //reverse the above travel sequence to get home (travellers need a)where home is b)what country they are in.

      //if (travelList::travelLocations.find("London") == travelList::travelLocations.end()) return;//didn't find the holiday destination
      //if(travelList::travelLocations["London"]->isOnRemoteDomain())setRemoteLocation();
      if (_locationIsRemote)leaveDomain();
      placeCache[vehicle]=places[vehicle];//store current values to be restored after trip - NB do this *BEFORE* visit! 
      placeCache[home]=places[home];
      outwardTravel();
      currentPlace=vehicle;//now plane
}
//------------------------------------------------------------------------
void agent::returnFromHoliday(){
    if (timeStep::getMonth()==5 && timeStep::getDayOfMonth()==14) {//return from holiday after two weeks, at midnight
            if (_locationIsRemote)leaveDomain();
            inwardTravel();
    }
}
//------------------------------------------------------------------------
void agent::arriveHome(){
    if (timeStep::getMonth()==5 && timeStep::getDayOfMonth()==14 && timeStep::getTimeOfDay()>=1200){//got off the plane.
            setTransport(placeCache[vehicle]);
            currentPlace=home;
    }
}
//------------------------------------------------------------------------
void agent::setRemoteLocation(){
    _locationIsRemote=true;
}
//------------------------------------------------------------------------
void agent::inwardTravel(){//note this and outward travel below are used in the case of multiple MPI domains, so need to be kept separate from update travel schedule above
    //unstack home
    //setHome(placeCache[home]);
}
//------------------------------------------------------------------------
void agent::outwardTravel(){
    //travelList::travelLocations["London"]->visit(this);//if leaving domain this caches the return flight - note alters home and vehicle settings
}

//------------------------------------------------------------------------
void agent::initialize(parameterSettings& params){       
   //mobility defined by by the parameter file - should be consistent with modelfactory settings
    if      (params("model.type")=="simpleOnePlace"  )_mobile=false;
    else if (params("model.type")=="simpleMobile"      )_mobile=true;
}
//------------------------------------------------------------------------
void agent::cough()
{
        //breathInto(place) - scales linearly with the time spent there (using uniform timesteps) - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        //check first that the places has been defined properly.
        assert(places[currentPlace]!=nullptr);
        
        if (diseased()) places[currentPlace]->increaseContamination(disease::shedInfection());
}

//static variables have to be defined outside the header file
unsigned long agent::nextID=0;

