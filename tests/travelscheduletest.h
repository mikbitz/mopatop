#ifndef TRAVELSCHEDULETEST_H_INCLUDED
#define TRAVELSCHEDULETEST_H_INCLUDED
#include"../travelschedule.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class travelScheduleTest : public CppUnit::TestFixture  {
    travelSchedule* t;
public:
    //persistent objects to use during testing
    void setUp()
    {
        t = new travelSchedule();
    }
    
    void tearDown() 
    {
        delete t;
    }
    //automatically create a test suite to add tests to - note this has to come after any setup/tearDown
    CPPUNIT_TEST_SUITE( travelScheduleTest  );
    //add tests defined below
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testEmpty );
    CPPUNIT_TEST( testSwitch );
    CPPUNIT_TEST( testMobile );    
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaults()
    {
        //stationary at home schedule
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
        t->getNextLocation();
        CPPUNIT_ASSERT(t->getTimeAtCurrentPlace()==24*timeStep::hour());
        CPPUNIT_ASSERT(t->getCurrentDestination()==agent::home);
    }
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
    void testSwitch()
    {
        //unknown name ignored - NB each test contrsucts a new object of type t (so initial schedule is stationary)
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
