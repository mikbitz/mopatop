//compile with g++ -std=c++17 -lcppunit ../timestep.cpp ../places.cpp ../disease.cpp testing.cpp
#include <iostream>
#include <string>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TextTestProgressListener.h>
#include "../places.h"
#include "../timestep.h"
#include "../agent.h"
#include "../randomizer.h"
#include<math.h>
class randomTest : public CppUnit::TestFixture  {
    place* p;
public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( randomTest );
    //add tests defined below
    CPPUNIT_TEST( testSeed );
    CPPUNIT_TEST( testDistrib );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testSeed()
    {
        randomizer r,k;
        CPPUNIT_ASSERT(r.number()==k.number());
        r.setSeed(10);
        CPPUNIT_ASSERT(r.number()!=k.number());
        //k still out of step with r
        k.setSeed(10);
        CPPUNIT_ASSERT(r.number()!=k.number());
        randomizer u(10);
        u.number();//u should now be in step with k
        CPPUNIT_ASSERT(u.number()==k.number());
        //now both r and k back in step
        k.setSeed(1019377);r.setSeed(1019377);
        CPPUNIT_ASSERT(r.number()==k.number());
    }
    void testDistrib()
    {
        randomizer r(17);
        bool f=true;
        //test strictly between 0 and 1
        for (int i=0;i<10000;i++)f=f&&(r.number()<1)&&(r.number()>0);
        CPPUNIT_ASSERT(f);
        //mean should converge on 0.5?? or at least sqrt n fluctuations?
        double p=0,q=0;
        for (int i=0;i<10000;i++)p+=r.number()/10000;
        for (int i=0;i<1000000;i++)q+=r.number()/1000000;
        CPPUNIT_ASSERT(std::abs(0.5-q)<std::abs(0.5-p));
    }

};
class diseaseTest : public CppUnit::TestFixture  {
    place* p;
public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( diseaseTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testRecover );
    CPPUNIT_TEST( testDie );
    CPPUNIT_TEST( testInfection );
    CPPUNIT_TEST( testShed );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaults()
    {
        CPPUNIT_ASSERT(0.0004==disease::getRecoveryRate());
        CPPUNIT_ASSERT(0.1==disease::getDeathRate());
        CPPUNIT_ASSERT(0.002==disease::getShed());
    }
    void testDefaultConstructor()
    {
        disease d;
        CPPUNIT_ASSERT(0.0008==disease::getRecoveryRate());
        CPPUNIT_ASSERT(0.000==disease::getDeathRate());
        CPPUNIT_ASSERT(0.001==disease::getShed());
    }
    void testRecover()
    {
        disease d;
        //r and k have the same random sequence
        randomizer r,k;
        //default timestep should be 1 hour
        timeStep::setdeltaT(timeStep::hour());
        bool rec=(disease::getRecoveryRate()>k.number());
        bool t=disease::recover(r);
        CPPUNIT_ASSERT(t==rec);
        //set timestep to much greater than an hour
        timeStep::setdeltaT(timeStep::year());
        t=disease::recover(r);
        rec=(disease::getRecoveryRate()*24*365>k.number());
        CPPUNIT_ASSERT(t==rec);
    }
    void testDie()
    {
        disease d;
        //r and k have the same random sequence
        randomizer r,k;
        //default timestep should be 1 hour
        timeStep::setdeltaT(timeStep::hour());
        bool rec=(disease::getDeathRate()>k.number());
        bool t=disease::die(r);
        CPPUNIT_ASSERT(t==rec);
        //set timestep to much greater than an hour
        timeStep::setdeltaT(timeStep::year());
        t=disease::die(r);
        rec=(disease::getDeathRate()*24*365>k.number());
        CPPUNIT_ASSERT(t==rec);
    }
    void testInfection()
    {
        disease d;
        //r and k have the same random sequence
        randomizer r,k;
        //default timestep should be 1 hour
        timeStep::setdeltaT(timeStep::hour());
        bool rec=(0.000001>k.number());
        bool t=disease::infect(0.000001,r);
        CPPUNIT_ASSERT(t==rec);
        //set timestep to much greater than an hour
        timeStep::setdeltaT(timeStep::year());
        t=disease::infect(1./24/365,r);
        rec=(1>k.number());
        CPPUNIT_ASSERT(t==rec);
    }
    void testShed()
    {
        disease d;
        //default timestep should be 1 hour
        timeStep::setdeltaT(timeStep::hour());
        CPPUNIT_ASSERT(disease::getShed()==disease::shedInfection());
        timeStep::setdeltaT(timeStep::day());
        CPPUNIT_ASSERT(disease::getShed()*24==disease::shedInfection());
    }
};
//fixtures allow setting up and tearing down of objects
class placeTest : public CppUnit::TestFixture  {
    place* p;
public:
    //persistent objects to use during testing
    void setUp()
    {
        p = new place();
    }
    
    void tearDown() 
    {
        delete p;
    }
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( placeTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testID );
    CPPUNIT_TEST( testContamination );
    CPPUNIT_TEST( testUpdate );
    CPPUNIT_TEST( testAgents );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        CPPUNIT_ASSERT(0==p->getNumberOfOccupants());
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        CPPUNIT_ASSERT(std::abs(0.1-p->getFractionalDecrement())<10*std::numeric_limits<double>::min());
        CPPUNIT_ASSERT(!p->getCleanEveryStep());
        p->setCleanEveryStep();
        CPPUNIT_ASSERT(p->getCleanEveryStep());
        p->unsetCleanEveryStep();
        CPPUNIT_ASSERT(!p->getCleanEveryStep());
    }
    void testID()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        p->setID(73);
        CPPUNIT_ASSERT(73==p->getID());
    }
    void testContamination()
    {
        p->increaseContamination(0.1);
        //make sure value is close to 0.1 - because we are using doubles, it might not be exaclty equal
        CPPUNIT_ASSERT(std::abs(0.1-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->increaseContamination(0.1);
        CPPUNIT_ASSERT(std::abs(0.2-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->cleanContamination();
        //should be nearly zero
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //negatives get reset to zero.
        p->increaseContamination(-0.1);
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
    }
    void testUpdate()
    {
        p->increaseContamination(0.1);
        //make sure value is close to eseentially zero - because we are using doubles, it might not be exaclty equal
        p->setCleanEveryStep();
        p->update();
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->unsetCleanEveryStep();
        p->increaseContamination(0.1);
        p->setFractionalDecrement(0.0);
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->setFractionalDecrement(0.5);
        p->update();
        //timestep is 1 hour by default.
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //set timestep to half an hour
        timeStep::setdeltaT(0.5*timeStep::hour());
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)*exp(-0.5*0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
    }
    void testAgents()
    {
        agent* a=new agent();
        p->add(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->add(a);
        //agents are unique
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        //safe to remove non existent agent?
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        agent* b=new agent();
        p->remove(b);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        //now multiple (non-unique!) adds
        p->add(a);
        p->add(a);
        p->add(b);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==2);
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->add(a);
        a->setID(10);
        b->setID(15);
        //test the method to show number of occupants (default no listing)
        p->show();
        //list occupant IDs
        p->show(true);
        
    }
};

//report the start of each test
class MyCustomProgressTestListener : public CppUnit::TextTestProgressListener {
 public:
     virtual void startTest(CppUnit::Test *test) {
         std::cout<<"\n starting test "<< test->getName()<<std::endl;
     }
};
int main(){
  CppUnit::TextUi::TestRunner runner;
  //add the test reporting
  MyCustomProgressTestListener progress;
  runner.eventManager().addListener(&progress);
  //add test suites
  runner.addTest( placeTest::suite() );
  runner.addTest( diseaseTest::suite() );
  runner.addTest( randomTest::suite() );
  //run all test suites
  runner.run();
  return 0;
}
