#include"agent.h"
#include "travelschedule.h"
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//defined here so as to be after travelSchedule
void agent::update()
{
        //Use the base travel schedule - initialised at home for everyone
        counter-=timeStep::deltaT();//reduce by actual time represented by this timestep (since schedule is defined in hours rater than timesteps)
        if (counter<=0){
            currentPlace=schedule->getNextLocation();
            counter=schedule->getTimeAtCurrentPlace();
        }
        //expensive - only needed if agents need direct agent-to-agent interactions in a place -
        //might be made cheaper by alowing agents to be present in multiple places, but only active in one.
        //(this could allow for remote meetings/phone calls?!)
        //moveTo(currentPlace);
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
}
void agent::initTravelSchedule(){       
   schedule=new  travelSchedule();
   currentPlace=schedule->getNextLocation();
   counter=schedule->getTimeAtCurrentPlace();
}
void agent::cough()
{
        //breathInto(place) - scales linearly with the time spent there (using uniform timesteps) - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        if (diseased) places[currentPlace]->increaseContamination(disease::shedInfection());
}
