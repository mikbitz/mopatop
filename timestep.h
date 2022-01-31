#ifndef TIMESTEP_H_INCLUDED
#define TIMESTEP_H_INCLUDED
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
 * @file timestep.h 
 * @brief File containing the definition of the timeStep class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 * Modified 25/1/21 to include dates
 **/

#include<string>
#include<map>
#include"parameters.h"
/** @brief A static class to set up the real-world times that apply to a timestep
*   @details The idea here is that the code will use a timestep in seconds, but the user need not know this.\n
*   They can set values using a chosen time unit, and get the right number of timesteps in those using this class\n
*   The value of any time can be set in the relevant units by using the static variables e.g. to get a time of 8 hours
*\code
*double time_needed=8*timeStep::hour()
*\endcode
* Similarly, to add the current value of the timestep to a given time, use timestep::deltaT()
* \code
* double time= 4* timeStep::hour() + timeStep::deltaT()
* \endcode
* Configure this once at the start of the code by creating an object and feeding it the \ref parameterSettings object
 * \code
 * parameterSettings parameters;
 * parameters.readParameters("../defaultParameterFile");
 * timeStep t(parameters);
 * \endcode 
 The model stepNumber can be used to calculate the number of hours and minutes, and the weekday since the start of the model run.\n
 Each timestep the model updates the stepnumber held here for this purpose, so that  the static stepNumber variable always holds the value of \n
 the model step number, and this can be accessed from anywhere in the code. This is done by calling the timestep::update() method each timestep.\n
 This adds one to the stepNumber, and then calculates the date given the starting date for the model (defaults to Mon 1 jan 1900, as a known day).\n
 Days and monthsminutes and hours  are held as integers starting at zero to make date calculations easier.\n
 At present there are no time zones available, so dates are calculated in nominal UTC (ignoring leap seconds, but allowing for leap years)\n
 Similarly dates before 1752 are not correct as these predate the switch to the gregorian calendar.\n
 The default initial date is Mon. 1 Jan. 1900.\n
 NB Although one coudl use the ctime library, this proved to be unreliable in converting back and forth between dates in a tm structure and\n
 seconds since 1970 that are held in a time_t, both in terms of results not always being consistent and interms of calls to gmtime modifying\n
 tm pointers not involved in the call. :(\n
 An alternative would be to use the boost posix time and gregorian libraries, which work well, but boost can be a problem to compile against\n
 as libraries can vary between systems. Maybe fixed by C++ 20 chrono?
 */ 
class timeStep{
    /** @brief number of seconds in a year */
    static double years;
    /** @brief number of seconds in a month, approximately
        @details seconds in a year divided by 12 - used only if a "monthly" timestep is requested */
    static double months;
    /** @brief number of seconds in a day */
    static double days;
    /** @brief number of seconds in an aah */
    static double hours;
    /** @brief number of seconds in a minute*/
    static double minutes;
    /** @brief number of seconds in a second (!) */
    static double seconds;
    /** @brief number of seconds in a timestep*/
    static double dt;
    /** @brief Units for the timestep  - number of seconds in \ref dt will be set as required.
        @details can be years,months,days,hours,minutes or seconds*/
    static std::string units;
    /** @brief the current model step - updated in ther step method of model.h every timestep */
    static int stepNumber;
    /** @brief the days in each month, for use in calculating dates - values for currentDayOfMonth range from 0 to monthDays[month]-1 */
    static int monthDays[12];
    /** @brief The month of the year at the current step, from 0 (Jan.) to 11 (Dec.) */
    static int currentMonth;
    /** @brief The day of the month, starting from 0 */
    static int currentDayOfMonth;
    /**  @brief The day of the week from 0-6 with Monday as 0 */
    static int currentWeekDay;
    /** @brief The year expected to be a four digit integer */
    static int currentYear;
    /** @brief the hour of the day from 0 to 23 - expected to be a two digit integer */
    static int currentHour;
    /** @brief the minute of the hour from 0 to 59 */
    static int currentMinute;
    /** @brief the seconds of the hour from 0 to 59 */
    static int currentSeconds;
public:

    /** @brief Default constructor sets timestep units to be hours 
        @details The actual internal units here for timeStep is always seconds - here dt is set to be 3600, i.e. the defautl timestep is one hour*/
    timeStep(){
        units="hours";
        years   = 24*3600*365;//365 days in a year!!!
        months  = 24*3600*365/12;//every of these "months" has about 30.4 days- so these are not quite actual months - actual dates are found using the update method
        days    = 24*3600;
        hours   = 3600;
        minutes = 60;
        seconds = 1;
        dt=3600;

    }
    /** @brief Constructor to get the values from a \ref parameterSettings object 
     *  @param p a reference to a \ref parameterSettings object*/
    timeStep(parameterSettings& p){
        units     = p.get("timeStep.units");
        //make sure the string is all lower case and has no leadin gor trai ling spaces
        std::for_each(units.begin(), units.end(), [](char & c) {c = std::tolower(c);});
        units.erase(std::remove_if(units.begin(), units.end(), ::isspace), units.end());
        //get the base level for the above units
        dt        = p.get<double>("timeStep.dt");
        //set dt so that units are consistently seconds in this class
        //this way any other class can get the actual time represented by dt using the access functions below
        if        (units=="years"){ //number of timeSteps in one year = timeStep::years/timeStep::dt, and so on
            dt          *= years;
        }else  if (units=="months"){
            dt          *= months;
        }else  if (units=="days"){
            dt          *= days;
        }else  if (units=="hours"){
            dt          *= hours;
        }else  if (units=="minutes"){
            dt          *= minutes;
        }else  if (units=="seconds"){
            dt          *= seconds;
        }else {
            std::cout<<"Invalid time units: "<<units<<" in timeStep.h"<<std::endl;
            exit(1);
        }
        setDate(p.get("timeStep.startdate"));
    }
    //------------------------------------------------------------------------
    /** @brief set the timestep unit 
        @param u a string, one of years,months,days,hours,minutes,seconds*/
    static void setTimeStepUnit(std::string u){
        //ensure lower case and no spaces
        std::for_each(u.begin(), u.end(), [](char & c) {c = std::tolower(c);});
        u.erase(std::remove_if(u.begin(), u.end(), ::isspace), u.end());
        //since dt is always in seconds, no re-scaling is needed - the unit is really only needed at setup

        if (u=="years" ||  u=="months"|| u=="days"|| u=="hours"|| u=="minutes"|| u=="seconds"){
            units=u;
        }else {
            std::cout<<"Invalid time units: "<<units<<" in timeStep.h"<<std::endl;
            exit(1);
        }
        
    }
    //------------------------------------------------------------------------
    /** @brief report the timestep unit currently in use */
    static std::string timeStepUnit(){
        return units;
    }
    //------------------------------------------------------------------------
    /** @brief set the number of model steps since the start of the run, and calculate the date */
    static void update(){
        stepNumber++;
        currentSeconds+=deltaT();//deltaT is always in seconds
        if (currentSeconds>=60){
            currentMinute+=currentSeconds/60;
            currentSeconds=currentSeconds%60;
        }
        if (currentMinute>=60){
            currentHour+=currentMinute/60;
            currentMinute=currentMinute%60;
        }
        if (currentHour>=24){
            currentWeekDay+=(currentHour/24);
            currentWeekDay=currentWeekDay%7;
            currentDayOfMonth+=currentHour/24;
            currentHour=currentHour%24;
        }
        int leapday=0;
        if (currentDayOfMonth>=monthDays[currentMonth]){
            if (currentMonth==1){//February, since months run from 0 to 11
                //leap year if divisible by 4 unless a century in which case needs to be divisible by 400
                if(currentYear%400==0 || (currentYear%4==0 && currentYear%100!=0))leapday=1;
            }
            while(currentDayOfMonth>=monthDays[currentMonth]+leapday){
                currentDayOfMonth-=monthDays[currentMonth]+leapday;
                currentMonth++;
                if (currentMonth!=1) leapday=0;
            }
        }
        if (currentMonth>=12){
            currentYear+=currentMonth/12;
            currentMonth=currentMonth%12;
        }
        reportDate();
    }
    //------------------------------------------------------------------------
    /** @brief set the number of model steps since the start of the run   */
    static void setStepNumber(int s){
        stepNumber=s;
    }
    //------------------------------------------------------------------------
    /** @brief return the current number of model steps since the start of the run  
        @details It is assumed this will be updated every model timestep */
    static int getStepNumber(){
        return stepNumber;
    }
    //------------------------------------------------------------------------
    /** @brief return a representation of the time of day as in 24 hour clock e.g. 914 for for 14 minutes past nine in the morning  
     It is assumed that the model run starts at midnight i.e. step 0 is 0000h */
    static int getTimeOfDay(){
        return currentHour*100+currentMinute;
    }
    //------------------------------------------------------------------------
    /** @brief return a representation of the day of the week as an integer with 0=Mon, 1=Tue etc.  
      The model run is assumed to start on a Monday by default */
    static int getDayOfWeek(){
        return currentWeekDay;
    }
    //------------------------------------------------------------------------
    /** @brief return a representation of the month of the year as an integer with 0=Jan, 1=Feb etc.  
      The model run is assumed to start on 1 January by default */
    static int getMonth(){
        return currentMonth;
    }
    //------------------------------------------------------------------------
    /** @brief report the values in the current date structure in format: Day dd/mm/yyyy hh:mm:ss*/
    static void reportDate(){
        std::string wday[7]={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
        std::cout<<wday[currentWeekDay]<<" ";
        if (currentDayOfMonth<10) std::cout<<"0";
        std::cout<<currentDayOfMonth+1<<"/";
        if (currentMonth<10) std::cout<<"0";
        std::cout<<currentMonth+1<<"/";
        std::cout<<currentYear<<" ";
        if (currentHour<10) std::cout<<"0";
        std::cout<<currentHour<<":";
        if (currentMinute<10) std::cout<<"0";
        std::cout<<currentMinute<<":";
        if (currentSeconds<10) std::cout<<"0";
        std::cout<<currentSeconds<<":";
    }
    //------------------------------------------------------------------------
    /** @brief Set the values in the date structure - Note apart from the year all values are integers starting from zero 
     @param year The year as a four digit integer
     @param month the month as an integer with 0=Jan. 1=Feb. etc
     @param dayofweek The day of the week with 0=Mon. 1=Tue. up to 6=Sun.
     @param monthday The day of the month - from 0 to some upper value depending on the month
     @param hour Hour of the day from 0 to 23
     @param min minute of the hour from 0 to 59
     @param sec second from 0 59
     @details Some minimal checking is carried out but reliance is placed on the user to get all the details here correct! e.g. day of the week consistent with other values
     */
    static void setDate(int year,int month,int dayofweek,int monthday,int hour,int min,int sec){
        assert(month >=0 && month<12);
        assert(dayofweek>=0 && dayofweek<7);
        assert(monthday>=0 && monthday<monthDays[month]);
        assert(hour>=0 && hour<24);
        assert(min>=0 && min<60);
        assert(sec>=0 && sec<60);
        currentWeekDay=dayofweek;
        currentDayOfMonth=monthday;
        currentMonth=month;
        currentYear=year;
        currentHour=hour;
        currentMinute=min;
        currentSeconds=sec;

    }
        //------------------------------------------------------------------------
    /** @brief Set the values in the date structure - using input in the form of a string: Day dd/mm/yyyy hh:mm:ss e.g. "Mon 01/01/1900 00:00:00"
     @param s the date in the above format - note spaces slashes and colons expected exactly as shown (no extra . or anything else)
     @details Some minimal checking is carried out but reliance is placed on the user to get all the details here correct! e.g. day of the week consistent with other values
     */
    static void setDate(std::string s){
        std::map<std::string,int> wday;
        wday["Mon"]=0;
        wday["Tue"]=1;
        wday["Wed"]=2;
        wday["Thu"]=3;
        wday["Fri"]=4;
        wday["Sat"]=5;
        wday["Sun"]=6;
        int pos;
        std::string w;

        pos=s.find(" ");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        assert(wday.find(w)!=wday.end());
        int dayofweek=wday[w];
        
        pos=s.find("/");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        int monthday=std::stoi(w)-1;//days of month normally quoted beginning at 1 but internally we use 0
        
        pos=s.find("/");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        int month=std::stoi(w)-1;//months normally quoted beginning at 1 from Jan. but internally we use 0
        
        pos=s.find(" ");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        int year=std::stoi(w);
        
        pos=s.find(":");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        int hour=std::stoi(w);

        pos=s.find(":");
        w=s.substr(0,pos);
        s.erase(0,pos+1);
        int min=std::stoi(w);
        
        // seconds should be all that's left!
        int sec=std::stoi(s);

        setDate(year,month,dayofweek,monthday,hour,min,sec);
    }
    //------------------------------------------------------------------------
    /** @brief set the timestep value in seconds   */
    static void setdeltaT(double sec){
        dt=sec;
    }
    //------------------------------------------------------------------------
    /** @brief report the timestep value in seconds  */
    static double deltaT(){
        return dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for a (365 day )year  */
    static double year(){
        return years;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for a nominal (30 day) month  */
    static double month(){
        return months;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for a day   */
    static double day(){
        return days;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for an hour   */
    static double hour(){
        return hours;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for a minute */
    static double minute(){
        return minutes;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds for a second  */
    static double second(){
        return seconds;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of years that would be represented by a timestep 
        @details e.g if the time step is one day, report 1./365 */
    static double yearsPerTimeStep(){
        return dt/years;
    }
    //------------------------------------------------------------------------
   /** @brief report the number of months that would be represented by a timestep 
       @details e.g if the time step is one day, report 1./30  (so not really months!)*/
    static double monthsPerTimeStep(){
        return dt/months;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of days that would be represented by a timestep 
       @details e.g if the time step is two days, report 2.  */
    static double daysPerTimeStep(){
        return dt/days;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of hours that would be represented by a timestep */
    static double hoursPerTimeStep(){
        return dt/hours;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of minutes that would be represented by a timestep */
    static double minutesPerTimeStep(){
        return dt/minutes;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of seconds that would be represented by a timestep */
    static double secondsPerTimeStep(){
        return dt/seconds;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into a (365 day) year */
    static double TimeStepsPerYear(){
        return years/dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into a nominal (30 day) month */
    static double TimeStepsPerMonth(){
        return months/dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into a day */
    static double TimeStepsPerDay(){
        return days/dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into an aah */
    static double TimeStepsPerHour(){
        return hours/dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into a minute */
    static double TimeStepsPerMinute(){
        return minutes/dt;
    }
    //------------------------------------------------------------------------
    /** @brief report the number of timesteps that would fit into a second */
    static double TimeStepsPerSecond(){
        return seconds/dt;
    }
};

#endif // TIMESTEP_H_INCLUDED
