#ifndef SCHEDULELIST_H_INCLUDED
#define SCHEDULELIST_H_INCLUDED
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
 * @file scheduleList.h 
 * @brief File containing the definition of a class to hold a list of travel schedules
 * 
 * @author Mike Bithell
 * @date 26/10/2021
 **/

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief A list of pre-defined travel schedules
 * @details Agents can use this list to switch 

*/
#include<map>
#include<string>
class travelSchedule;
class scheduleList{

public:
    /** @brief This enum associates a set of integers with names. 
     * @details So stationary=0, mobile=1 etc. This allows meaningful names to be used to refer to the type of schedule, for example.*/
    enum scheduleTypes{stationary,mobile,remoteTravel,returnTrip};
    /** @brief Constructor to build the list of schedules 
     *  @details add the schduleTypes to the schedules vector
     */
    scheduleList();
    /** return one of the travel schedules in the list by using its type */
    travelSchedule& operator[](scheduleTypes i);
    /** convert a string schedule name (e.g. from the parameter file) to one of the schedule type enums */
    scheduleTypes getType(std::string scheduleType);
    /** return a scheduleType correspoding to a given named string. */
    std::string getName(scheduleList::scheduleTypes sT);
private:
    /** A map to hold a list of travel schedules, indexed by type */
    std::map<scheduleTypes,travelSchedule> schedules; 
    
};
#endif // SCHEDULELIST_H_INCLUDED
