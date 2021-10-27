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
    /** @brief set up a default travel schedule pointer (stationary at home) */
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
    /** @brief test alternate constructors using types  */
    CPPUNIT_TEST( testType);
    /** @brief end the test suite */
    CPPUNIT_TEST_SUITE_END();
     /** @brief check the default schedule is as expected - make sure the destinations and timings are correct */
    void testDefaults()
    {
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(0)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(0)==agent::home);
        t->getNextLocation(0);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(1)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(1)==agent::home);
    }
     /** @brief make sure the schedule behaves smoothly if empty (the agent stays at home the whole time */
    void testEmpty()
    {
        //clean the schedule - empty schedule should not crash!
        t->cleanOldSchedule();
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(0)==0);
        CPPUNIT_ASSERT(t->getCurrentDestination(0)==agent::home);
        t->getNextLocation(1);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(1)==0);
        CPPUNIT_ASSERT(t->getCurrentDestination(1)==agent::home);
    } 
    /** @brief check that incorrect schedule names have no effect and check switching works . 
     * @details Switch to a mobile schedule and run through it a bit, then switch back to stationary \n
     */
    void testSwitch()
    {
        //test error in switch string
        t->switchTo("mbile");
        //should still have stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(0)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(0)==agent::home);
        t->getNextLocation(0);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(5)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(5)==agent::home);
        t->switchTo("mobile");
        //moving!
        CPPUNIT_ASSERT(t->getStartPoint()==2);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(2)==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(2)==agent::vehicle);
        t->getNextLocation(1);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(3)==14*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(3)==agent::home);
        t->switchTo("stationary");
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(0)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(0)==agent::home);
        t->getNextLocation(70);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(85)==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(103)==agent::home);
    }
    /** @brief exercise the mobile schedule 
     * @details run completely through the schedule making sure it wraps round to the start again after a day \n
     */
    void testMobile()
    {
        //stationary at home schedule switches to mobile
        t->switchTo("mobile");
        //mobile schedule
        CPPUNIT_ASSERT(t->getStartPoint()==2);
        unsigned point=2;
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(point)==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(point)==agent::vehicle);
        point=t->increment(point);
        CPPUNIT_ASSERT(point==3);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(point)==14*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(point)==agent::home);
        point=t->increment(point);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(point)==1*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(point)==agent::vehicle);
        point=t->increment(point);
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace(point)==8*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination(point)==agent::work);
    }
    /** @brief check that the mobile and stationary schedules are picked up correctly in the constructor
        @details the non-default constructor should use the type from schedule list to pick out one of the defined schedules*/
    void testType()
    {
        //stationary at home schedule
        travelSchedule m(scheduleList::mobile);
        //mobile schedule
        unsigned point=m.getStartPoint();
        CPPUNIT_ASSERT(point==2);
        CPPUNIT_ASSERT(m.getTimeAtCurrentPlace(point)==1*timeStep::hour());
        CPPUNIT_ASSERT(m.getCurrentDestination(point)==agent::vehicle);
        point=m.increment(point);
        CPPUNIT_ASSERT(point==3);
        CPPUNIT_ASSERT(m.getTimeAtCurrentPlace(point)==14*timeStep::hour());
        CPPUNIT_ASSERT(m.getCurrentDestination(point)==agent::home);
        point=m.increment(point);
        CPPUNIT_ASSERT(m.getTimeAtCurrentPlace(point)==1*timeStep::hour());
        CPPUNIT_ASSERT(m.getCurrentDestination(point)==agent::vehicle);
        point=m.increment(point);
        CPPUNIT_ASSERT(m.getTimeAtCurrentPlace(point)==8*timeStep::hour());
        CPPUNIT_ASSERT(m.getCurrentDestination(point)==agent::work);
        travelSchedule s(scheduleList::stationary);
        //stationary at home schedule
        CPPUNIT_ASSERT(s.getTimeAtCurrentPlace(0)==24*timeStep::hour());
        CPPUNIT_ASSERT(s.getCurrentDestination(0)==agent::home);
        s.getNextLocation(1);
        point=s.getStartPoint();
        CPPUNIT_ASSERT(point==0);
        point=s.increment(point);
        CPPUNIT_ASSERT(point==0);
        CPPUNIT_ASSERT(s.getTimeAtCurrentPlace(point)==24*timeStep::hour());
        CPPUNIT_ASSERT(s.getCurrentDestination(point)==agent::home);
    }
};
#endif // TRAVELSCHEDULETEST_H_INCLUDED
