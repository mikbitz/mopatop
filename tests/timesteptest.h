#ifndef TIMESTEPTEST_H_INCLUDED
#define TIMESTEPTEST_H_INCLUDED
#include"../timestep.h"
/* A program to test the model of agents moving between places
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
 * @file timesteptest.h 
 * @brief File containing the definition of the timeStepTest class for checking the time step to real world times class is working
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief Check the class that maps the time step to a given set of real-world units (e.g. hours)
 *  @details NB if timeStep::deltaT is reset anywhere, because timestep is a static class, this can affect other tests
 * */

class timeStepTest : public CppUnit::TestFixture  {
public:
    /** @brief automatically create a test suite to add tests to - note this has to come after any setup/tearDown */
    CPPUNIT_TEST_SUITE( timeStepTest );
    /** @brief test the default state */
    CPPUNIT_TEST( testDefaults );
    /** @brief check the constructor works used on an object */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief check that all the functions related to the timestep work */
    CPPUNIT_TEST( testReturnValues );
    /** @brief check that all date functionality works */
    CPPUNIT_TEST( testDateFunctions );
    /** @brief end test suite */
    CPPUNIT_TEST_SUITE_END();
    /** @brief as a static class the static variable should have values as in timeStep.cpp */
    void testDefaults()
    {
        CPPUNIT_ASSERT(timeStep::year()==24*3600*365);
        CPPUNIT_ASSERT(timeStep::month()==24*30*3600);
        CPPUNIT_ASSERT(timeStep::day()==24*3600);
        CPPUNIT_ASSERT(timeStep::hour()==3600);
        CPPUNIT_ASSERT(timeStep::minute()==60);
        CPPUNIT_ASSERT(timeStep::second()==1);
        CPPUNIT_ASSERT(timeStep::deltaT()==3600);
        CPPUNIT_ASSERT(timeStep::timeStepUnit()=="hours");
    }
    /** @brief When an object is created, the values should be those from the constructor in timeStep.h */
    void testDefaultConstructor()
    {
        timeStep t;
        CPPUNIT_ASSERT(timeStep::year()==24*3600*365);
        CPPUNIT_ASSERT(timeStep::month()==24*3600*365/12);
        CPPUNIT_ASSERT(timeStep::day()==24*3600);
        CPPUNIT_ASSERT(timeStep::hour()==3600);
        CPPUNIT_ASSERT(timeStep::minute()==60);
        CPPUNIT_ASSERT(timeStep::second()==1);
        CPPUNIT_ASSERT(timeStep::deltaT()==3600);
        CPPUNIT_ASSERT(timeStep::timeStepUnit()=="hours");
    }
    /** @brief Check that the return values are as expected 
     @details e.g. after setting timeStep::setdeltaT(timeStep::hour()), TimeStepsPerHour and hoursPerTimeStep should both be 1 */
    void testReturnValues(){
        //dt should at first be one hour
        CPPUNIT_ASSERT( timeStep::yearsPerTimeStep()  ==1./24/365);
        CPPUNIT_ASSERT( timeStep::monthsPerTimeStep()*double(24*365) ==12);//written this way to avoid rounding error
        CPPUNIT_ASSERT( timeStep::daysPerTimeStep()   ==1./24);
        CPPUNIT_ASSERT( timeStep::hoursPerTimeStep()  ==1);
        CPPUNIT_ASSERT( timeStep::minutesPerTimeStep()==60);
        CPPUNIT_ASSERT( timeStep::secondsPerTimeStep()==3600);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerYear()  ==24*365);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMonth() ==24*365/12);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerDay()   ==24);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerHour()  ==1);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMinute()==1./60);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerSecond()==1./3600);
        //change timestep to be a day
        timeStep::setdeltaT(timeStep::day());
        CPPUNIT_ASSERT( timeStep::yearsPerTimeStep()  ==1./365);
        CPPUNIT_ASSERT( timeStep::monthsPerTimeStep() ==12./365.);
        CPPUNIT_ASSERT( timeStep::daysPerTimeStep()   ==1.);
        CPPUNIT_ASSERT( timeStep::hoursPerTimeStep()  ==24);
        CPPUNIT_ASSERT( timeStep::minutesPerTimeStep()==24*60);
        CPPUNIT_ASSERT( timeStep::secondsPerTimeStep()==24*3600);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerYear()  ==365);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMonth() ==365/12.);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerDay()   ==1);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerHour()  ==1./24);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMinute()*24*60==1.);//written this way to avoid rounding error
        CPPUNIT_ASSERT( timeStep::TimeStepsPerSecond()==1./24/3600);
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
        /** @brief Check daet calculations 
     @details Check the weekday calcualtor works for some known days */
    void testDateFunctions(){
        //findWeekday uses months and days starting at 1
        CPPUNIT_ASSERT( timeStep::findWeekDay(1900,1,1)==0);//first day of the century was a Monday
        CPPUNIT_ASSERT( timeStep::findWeekDay(2022,2,23)==2);//A wednesday
        CPPUNIT_ASSERT( timeStep::findWeekDay(2021,12,17)==4);//A friday
        CPPUNIT_ASSERT( timeStep::findWeekDay(2020,5,10)==6);//A sunday
        CPPUNIT_ASSERT( timeStep::findWeekDay(1800,1,1)==2);//A wednesday
        CPPUNIT_ASSERT( timeStep::findWeekDay(1777,4,30)==2);//ditto
        CPPUNIT_ASSERT( timeStep::findWeekDay(1783,9,18)==3);//Thurs.
        CPPUNIT_ASSERT( timeStep::findWeekDay(1949,10,31)==0);//Mon.
        CPPUNIT_ASSERT( timeStep::findWeekDay(1952,10,11)==5);//Sat.
        //setDate call looks like setDate(year,month,dayofweek,monthday,hour,min,sec)
        //all *zero* based now, except year
        timeStep::setDate(1949,9,1,9,01,02,03);//should correct to day of week =0 (Monday)
        timeStep::reportDate();
        CPPUNIT_ASSERT( timeStep::getYear()==1949);
        CPPUNIT_ASSERT( timeStep::getMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==0);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==102);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //now test the string version
        timeStep::setDate("Mon 10/10/1949 01:02:03");
        //this should add one hour
        timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1949);
        CPPUNIT_ASSERT( timeStep::getMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==0);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==202);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //change timestep to be a day and add a day
        timeStep::setdeltaT(timeStep::day());
        timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1949);
        CPPUNIT_ASSERT( timeStep::getMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==10);
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==1);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==202);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //now try a year
        timeStep::setdeltaT(timeStep::year());
        timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1950);
        CPPUNIT_ASSERT( timeStep::getMonth()==9);
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==10);
        //not a leap year, so day advances by 1
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==2);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==202);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        timeStep::setdeltaT(2*timeStep::year());
        timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1952);
        CPPUNIT_ASSERT( timeStep::getMonth()==9);
        //1952 is a leap year, so day of month is one less
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==9);
        //day of week advances by 1 though!
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==4);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==202);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //check that days before feb are correct though
        timeStep::setDate("Thu 11/1/1951 11:02:03");
        timeStep::setdeltaT(timeStep::year());
        timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1952);
        CPPUNIT_ASSERT( timeStep::getMonth()==0);
        //1952 is a leap year, but we are before Feb.
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==10);
        //day of week advances by 1 still
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==4);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==1102);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //now advance one month in daily steps
        timeStep::setdeltaT(timeStep::day());
        //Jan. has 31 days
        for (int i=0;i<31;i++)timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1952);
        CPPUNIT_ASSERT( timeStep::getMonth()==1);
        //1952 is a leap year, but we are before Feb.
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==10);
        //day of week advances by 4 weeks 3 days
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==0);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==1102);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //forward a further year
        for (int i=0;i<365;i++)timeStep::update();
        CPPUNIT_ASSERT( timeStep::getYear()==1953);
        CPPUNIT_ASSERT( timeStep::getMonth()==1);
        //1952 is a leap year so one day back
        CPPUNIT_ASSERT( timeStep::getDayOfMonth()==9);
        //day of week advances by 1
        CPPUNIT_ASSERT( timeStep::getDayOfWeek()==1);
        CPPUNIT_ASSERT( timeStep::getTimeOfDay()==1102);
        CPPUNIT_ASSERT( timeStep::getSeconds()==3);
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
};
#endif // TIMESTEPTEST_H_INCLUDED
