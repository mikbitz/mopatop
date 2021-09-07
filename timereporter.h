#ifndef TIMEREPORTER_H_INCLUDED
#define TIMEREPORTER_H_INCLUDED
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
/**
 * @file timereporter.h 
 * @brief File containing the definition of a class used to wrap the C++ timing classes
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

#include<chrono>
#include<ctime>
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** 
 * @brief Simple static class to abstract the clunky C++ chrono system
 * @details This class allows a variable to be created that will store the current run-time at the point it is created. \n
 * A second call later on then allows for the elapsed time to be calculated and show with the \ref showInterval method. \n
 * For example:-
 * \code
 * auto start=timeReporter::getTime();
 * ... do some stuff ...
 * auto end=timeReporter::getTime();
 * showInterval("time taken was",start,end);
 * \endcode
 * Note the use of "auto" so that the datatype of start and end doesn't need to be remembered!
 */
class timeReporter{
public:
    /** get the time now */
    static std::chrono::time_point<std::chrono::steady_clock> getTime(){return std::chrono::steady_clock::now();}
    /** show the interval between two time points in seconds (to the nearest millsecond) using standard output to the terminal
     @param s A string containing the message to show describing this time interval
     @param start The start of the interval as reported by \ref getTime
     @param end The correspinding end of the interval
     */
    static void showInterval(std::string s,std::chrono::time_point<std::chrono::steady_clock> start,std::chrono::time_point<std::chrono::steady_clock> end){
        std::cout<<s<<float(interval(start,end))/1000<<" seconds"<<std::endl;
    }
    /** calculate the interval between two time points as an integer - use absolute value so that order of arguments doesn't matter
     @param start The start of the interval as reported by \ref getTime
     @param end The correspinding end of the interval
     @return a positive integer giving the interval in milliseconds
     */
    static unsigned interval(std::chrono::time_point<std::chrono::steady_clock> start,std::chrono::time_point<std::chrono::steady_clock> end){
        return abs(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count());
    }

};
#endif //TIMEREPORTER_H_INCLUDED
