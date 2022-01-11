#include"agent.h"
#include"places.h"
#include "travelschedule.h"
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
 * @brief File containing methods that requie definitions of other classes to be available
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#include"agent.h"
#include"places.h"
#include "travelschedule.h"
#include "remoteTravel.h"
#include "modelFactory.h"
#include"schedulelist.h"

//static list of travel destinations away from home
std::map<std::string,remoteTravel*> travelList::travelLocations;

//------------------------------------------------------------------------
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
//defined here so as to be after travelSchedule class
void agent::update(long step)
{
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver

        updateTravelSchedule(step);

        //moving agents between data structure is expensive - only needed if agents need direct agent-to-agent interactions in a place -
        //might be made cheaper by allowing agents to be present in multiple places, but only active in one.
        //(this could allow for remote meetings/phone calls?!)
        //moveTo(currentPlace);

}
//------------------------------------------------------------------------
void agent::updateTravelSchedule(long step)
{

    //are we about to reach end of schedule? - used for travel away from home
    if (scheduleTimer-timeStep::deltaT()<=0 && currentPlace==home){
        if (scheduleType==scheduleList::returnTrip   ){//returning from time away from home
            setTransport(placeCache[vehicle]);
            scheduleType=originalScheduleType;
            initTravelSchedule();
        }
        else if (scheduleType==scheduleList::remoteTravel ){//set up return trip unstack home - will be on local aeroplane, as set up by visit in outwardTravel.
            if (_locationIsRemote)leaveDomain();
            inwardTravel();
        }
        else if (holidayTime(step)                            ){//is the agent going to go to another destination?
            if (_locationIsRemote)leaveDomain();
            originalScheduleType=scheduleType; //cache schedule for return home
            placeCache[vehicle]=places[vehicle];//store current values to be restored after trip - NB do this *BEFORE* visit! 
            placeCache[home]=places[home];
            outwardTravel();
            //reset holiday time here??
        }else{
            advanceTravelSchedule();
        }
    }else{
        advanceTravelSchedule();
    }
}
//------------------------------------------------------------------------
bool agent::holidayTime(long step){
    //rule for deciding to go on holiday
    if (ID<=35000 && step < 30){
    if (travelList::travelLocations.find("London") == travelList::travelLocations.end()) return false;
    if(travelList::travelLocations["London"]->isOnRemoteDomain())setRemoteLocation();
    return true;
    }
    return false;
}
//------------------------------------------------------------------------
void agent::setRemoteLocation(){
    _locationIsRemote=true;
}
//------------------------------------------------------------------------
void agent::inwardTravel(){//note this and outward travel below are used in the case of multiple MPI domains, so need to be kept separate from update travel schedule above
    //unstack home
    setHome(placeCache[home]);
    initTravelSchedule("returnTrip");
}
//------------------------------------------------------------------------
void agent::outwardTravel(){
    travelList::travelLocations["London"]->visit(this);//if leaving domain this caches the return flight - note alters home and vehicle settings
    initTravelSchedule("remoteTravel");
}
//------------------------------------------------------------------------
void agent::advanceTravelSchedule(){
    //Use the base travel schedule - initialised at home for everyone
    scheduleTimer-=timeStep::deltaT();//reduce by actual time represented by this timestep (since schedule is defined in hours rater than timesteps)
    if (scheduleTimer<=0){

        currentPlace=allSchedules[scheduleType].getNextLocation(schedulePoint);
        //the agent controls whether to step to the next location on the schedule - default is to step forward - otherwise they may want to step back...
        //e.g by calling schedule->getcurrentDestination rather than incrementing schedulePoint.
        schedulePoint=allSchedules[scheduleType].increment(schedulePoint);
        scheduleTimer=allSchedules[scheduleType].getTimeAtCurrentPlace(schedulePoint);

    }
}
//------------------------------------------------------------------------
void agent::initTravelSchedule(parameterSettings& params){       
   //by default we go to the schedule defined by by the parameter file
   initTravelSchedule(params("schedule.type"));

}
//------------------------------------------------------------------------
void agent::initTravelSchedule(std::string s){       

   scheduleType=allSchedules.getType(s);
   initTravelSchedule();
}
//------------------------------------------------------------------------
void agent::initTravelSchedule(){       

   schedulePoint=allSchedules[scheduleType].getStartPoint();
   scheduleTimer=0;//assume we are at the end of the previous event in the schedule.
   currentPlace=allSchedules[scheduleType].getCurrentDestination(schedulePoint);
   //go to the start of the next event
   advanceTravelSchedule();
}
//------------------------------------------------------------------------
void agent::cough()
{
        //breathInto(place) - scales linearly with the time spent there (using uniform timesteps) - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        //check first that the places has been defined properly.
        assert(places[currentPlace]!=nullptr);
        
        if (diseased()) places[currentPlace]->increaseContamination(disease::shedInfection());
}
//------------------------------------------------------------------------
//static variables have to be defined outside the header file
unsigned long agent::nextID=0;
scheduleList agent::allSchedules;
