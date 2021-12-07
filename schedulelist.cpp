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
    schedules[stationary]   = travelSchedule(stationary);
    schedules[mobile]       = travelSchedule(mobile);
    schedules[remoteTravel] = travelSchedule(remoteTravel);
    schedules[returnTrip]   = travelSchedule(returnTrip);
}
travelSchedule& scheduleList::operator[](scheduleList::scheduleTypes i){
    return schedules[i];
}
scheduleList::scheduleTypes scheduleList::getType(std::string scheduleString){
    scheduleTypes s=stationary;
    if      (scheduleString=="stationary"  )s=stationary;
    else if (scheduleString=="mobile"      )s=mobile;
    else if (scheduleString=="remoteTravel")s=remoteTravel;
    else if (scheduleString=="returnTrip"  )s=returnTrip;
    else std::cout<<"Unknown schedule type:"<<scheduleString<<" in scheduleList::getType - defaulting to stationary"<<std::endl;
    return s;
}
std::string scheduleList::getName(scheduleList::scheduleTypes sT){
    std::string s="stationary";
    if      (sT==stationary)  s="stationary";
    else if (sT==mobile)      s="mobile";
    else if (sT==remoteTravel)s="remoteTravel";
    else if (sT==returnTrip)  s="returnTrip";
    else std::cout<<"Unknown schedule type:"<<sT<<" in scheduleList::getType - defaulting to stationary"<<std::endl;
    return s;
}
