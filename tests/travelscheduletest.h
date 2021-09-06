#ifndef TRAVELSCHEDULETEST_H_INCLUDED
#define TRAVELSCHEDULETEST_H_INCLUDED
#include"../travelschedule.h"
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
 * @file travelscheduletest.h 
 * @brief Test out the travel schedules using travelScheduleTest class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief Exercise the fixed travel schedules
 *  @details Check that switching between schedules resets each to the start.\n
 *  Check the destinations and timings are as expected.
 */
class travelScheduleTest : public CppUnit::TestFixture  {
    /** @brief A pointer to a default schedule for the setUp method */
    travelSchedule* t;
public:
    /** @brief set up a default travel schedule pointer */
    void setUp()
    {
        t = new travelSchedule();
    }
     /** @brief delete the schedule */
    void tearDown() 
    {
        delete t;
    }
    /** @brief automatically create a test suite to add tests to - note this has to come after any setup/tearDown */
    CPPUNIT_TEST_SUITE( travelScheduleTest  );
    /** @brief add test of defaults to the suite */
    CPPUNIT_TEST( testDefaults );
    /** @brief test of empty schedule */
    CPPUNIT_TEST( testEmpty );
    /** @brief test schedule switching */
    CPPUNIT_TEST( testSwitch );
    /** @brief test mobile schedule */
    CPPUNIT_TEST( testMobile );
    /** @brief end the test suite */
    CPPUNIT_TEST_SUITE_END();
     /** @brief check the default schedule is as expected - make sure the destinations and timings are correct */
    void testDefaults()
    {
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
    }
     /** @brief make sure the schedule behaves smoothly if empty (the agent stays at home the whole time */
    void testEmpty()
    {
        //clean the schedule - empty schedule should not crash!
        t->cleanOldSchedule();
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==0);
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==0);
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
    } 
    /** @brief check that incorrect schedule names have no effect and check switching works . 
     * @details Switch to a mobile schedule and run through it a bit, then switch back to stationary \n
     */
    void testSwitch()
    {

        t->switchTo("mbile");
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->switchTo("mobile");
        //moving!
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::vehicle);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==14*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->switchTo("stationary");
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
    }
            /** @brief exercise the mobile schedule 
         * @details run completely through the schedule making sure it wraps round to the start again after a day \n
           */
    void testMobile()
    {
        //stationary at home schedule
        t->switchTo("mobile");
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::vehicle);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==14*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::vehicle);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==8*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::work);
    }
};
#endif // TRAVELSCHEDULETEST_H_INCLUDED
