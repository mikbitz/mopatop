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
/** @brief Check the class that maps the time step to a given set of real-wprd units (e.g. hours)
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
    /** check that all the functions work */
    CPPUNIT_TEST( testReturnValues );
    /** end test suite */
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
        CPPUNIT_ASSERT(timeStep::month()==24*30*3600);
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
        CPPUNIT_ASSERT( timeStep::monthsPerTimeStep()*24*30 ==1.);//written this way to avoid rounding error
        CPPUNIT_ASSERT( timeStep::daysPerTimeStep()   ==1./24);
        CPPUNIT_ASSERT( timeStep::hoursPerTimeStep()  ==1);
        CPPUNIT_ASSERT( timeStep::minutesPerTimeStep()==60);
        CPPUNIT_ASSERT( timeStep::secondsPerTimeStep()==3600);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerYear()  ==24*365);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMonth() ==24*30);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerDay()   ==24);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerHour()  ==1);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMinute()==1./60);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerSecond()==1./3600);
        //change timestep to be a day
        timeStep::setdeltaT(timeStep::day());
        CPPUNIT_ASSERT( timeStep::yearsPerTimeStep()  ==1./365);
        CPPUNIT_ASSERT( timeStep::monthsPerTimeStep() ==1./30);
        CPPUNIT_ASSERT( timeStep::daysPerTimeStep()   ==1.);
        CPPUNIT_ASSERT( timeStep::hoursPerTimeStep()  ==24);
        CPPUNIT_ASSERT( timeStep::minutesPerTimeStep()==24*60);
        CPPUNIT_ASSERT( timeStep::secondsPerTimeStep()==24*3600);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerYear()  ==365);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMonth() ==30);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerDay()   ==1);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerHour()  ==1./24);
        CPPUNIT_ASSERT( timeStep::TimeStepsPerMinute()*24*60==1.);//written this way to avoid rounding error
        CPPUNIT_ASSERT( timeStep::TimeStepsPerSecond()==1./24/3600);
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
    
};
#endif // TIMESTEPTEST_H_INCLUDED
