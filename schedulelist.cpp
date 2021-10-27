//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @file schedulelist.cpp 
 * @brief File to define list of schedules for use by agents
 * 
 * @author Mike Bithell
 * @date 27/10/2021
 **/
#include "schedulelist.h"
#include "travelschedule.h"
scheduleList::scheduleList(){
    schedules[stationary]=travelSchedule(stationary);
    schedules[mobile]=travelSchedule(mobile);
}
travelSchedule& scheduleList::operator[](scheduleList::scheduleTypes i){
    return schedules[i];
}
scheduleList::scheduleTypes scheduleList::getType(std::string scheduleType){
    scheduleTypes s;
    if (scheduleType=="stationary")s=stationary;
    if (scheduleType=="mobile")s=mobile;
    return s;
}
