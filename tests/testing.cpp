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
//compile with g++ -std=c++17 -lcppunit ../timestep.cpp ../places.cpp ../disease.cpp testing.cpp
//Use the CPPUNIT testing suite to write unit tests for classes in the mopatop code base
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
#include "../travelschedule.h"
#include<math.h>
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class randomTest : public CppUnit::TestFixture  {
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
        //checking same seed gives same sequence
        //note setting seed implies sequence restart
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
        //now put both r and k back in step
        k.setSeed(1019377);r.setSeed(1019377);
        CPPUNIT_ASSERT(r.number()==k.number());
    }
    void testDistrib()
    {
        randomizer r(17);
        bool f=true;
        //test strictly between 0 and 1 - expecintg a uniform random distribution
        for (int i=0;i<10000;i++)f=f&&(r.number()<1)&&(r.number()>0);
        CPPUNIT_ASSERT(f);
        //mean should converge on 0.5?? or at least sqrt n fluctuations?
        //the variance should be smaller, the larger the number of samples, fo sure, you'd have thought...but seems not (settles at about 0.083)
        double p=0,q=0,s=0,l=0;
        for (int i=0;i<10;i++)p+=r.number()/10;
        for (int i=0;i<10;i++)s+=pow((r.number()-p),2)/10;
        for (int i=0;i<100000;i++)q+=r.number()/100000;
        for (int i=0;i<100000;i++)l+=pow((r.number()-q),2)/100000;
        CPPUNIT_ASSERT(std::abs(0.5-q)<0.1);
        CPPUNIT_ASSERT(std::abs(0.5-q)<std::abs(0.5-p));
        CPPUNIT_ASSERT(l<s);
        std::cout<<"Mean/Variance of 100,000 RNG samples: "<<q<<" "<<l<<std::endl;
    }

};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//NB if timeStep::deltaT is reset anywhere, because timestep is a static class, this can affect other tests
class timeStepTest : public CppUnit::TestFixture  {
public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( timeStepTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaults );
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testReturnValues );
    CPPUNIT_TEST_SUITE_END();
    //define tests
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class diseaseTest : public CppUnit::TestFixture  {
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
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
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
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
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
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
    void testShed()
    {
        disease d;
        //default timestep should be 1 hour
        timeStep::setdeltaT(timeStep::hour());
        CPPUNIT_ASSERT(disease::getShed()==disease::shedInfection());
        timeStep::setdeltaT(timeStep::day());
        CPPUNIT_ASSERT(disease::getShed()*24==disease::shedInfection());
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
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
        //timestep is 1 hour by default.
        timeStep::setdeltaT(timeStep::hour());
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //set timestep to half an hour
        timeStep::setdeltaT(0.5*timeStep::hour());
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)*exp(-0.5*0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
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
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class parameterTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( parameterTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testSet );
    CPPUNIT_TEST( testOperator );
    CPPUNIT_TEST( testReadWrite );
    CPPUNIT_TEST( testSetUpObjects );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p.get<int>("run.nSteps")==1);
        CPPUNIT_ASSERT(p.get<long>("run.nAgents")==600);
        CPPUNIT_ASSERT(p.get<int>("run.nThreads")==1);
        CPPUNIT_ASSERT(p.get<int>("run.randomSeed")==0);
        CPPUNIT_ASSERT(p.get("timeStep.units")=="hours");
        CPPUNIT_ASSERT(p.get<double>("timeStep.dt")==1);
        CPPUNIT_ASSERT(p.get("outputFile")=="diseaseSummary.csv");
        CPPUNIT_ASSERT(p.get("experiment.output.directory")=="./output");
        CPPUNIT_ASSERT(p.get("experiment.name")=="default");
        CPPUNIT_ASSERT(p.get<int>("experiment.run.number")==-1);
        CPPUNIT_ASSERT(p.get("experiment.description")=="The default parameter set was used");
        CPPUNIT_ASSERT(p.get<int>("experiment.run.prefix")==10000);
        CPPUNIT_ASSERT(p.get("model.version")=="Unknown");
        CPPUNIT_ASSERT(p.get<int>("run.nRepeats")==1);
        CPPUNIT_ASSERT(p.get<int>("run.randomIncrement")==1);
        CPPUNIT_ASSERT(p.get<double>("disease.simplistic.recoveryRate")==0.0007);
        CPPUNIT_ASSERT(p.get<double>("disease.simplistic.deathRate")==0.0007);
        CPPUNIT_ASSERT(p.get<double>("disease.simplistic.infectionShedLoad")==0.001);
        CPPUNIT_ASSERT(p.get<int>("disease.simplistic.initialNumberInfected")==1);
        CPPUNIT_ASSERT(p.get<double>("places.disease.simplistic.fractionalDecrement")==1);
        CPPUNIT_ASSERT(p.get<bool>("places.cleanContamination")==false);
        CPPUNIT_ASSERT(p.get("schedule.type")=="mobile");
        CPPUNIT_ASSERT(p.get("model.type")=="simpleMobile");
    }
    void testSet()
    {
        parameterSettings p;
        p.setParameter("model.type","test");
        CPPUNIT_ASSERT(p.get("model.type")=="test");
        CPPUNIT_ASSERT(p("model.type")=="test");
        p.setParameter("experiment.run.number","5");
        CPPUNIT_ASSERT(p.get<int>("experiment.run.number")==5);

    }
    void testOperator()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p("run.nSteps")=="1");
        CPPUNIT_ASSERT(p("experiment.run.number")=="-1");
        CPPUNIT_ASSERT(p("places.cleanContamination")=="false");

    }
    void testReadWrite()
    {
        parameterSettings p;
        //should print parameters to std::out
        p.readParameters("./testParameterFile");
        //should save parameters to a file called "RunParameters" in the current directory
        p.saveParameters("./");
        CPPUNIT_ASSERT(p.get<int>("run.nSteps")==3000);
        CPPUNIT_ASSERT(p.get<double>("timeStep.dt")==3600);
        CPPUNIT_ASSERT(p("experiment.run.number")=="-1");
        CPPUNIT_ASSERT(p("places.cleanContamination")=="true");
        CPPUNIT_ASSERT(p.get<int>("run.randomIncrement")==57);

    }
    void testSetUpObjects(){
        parameterSettings p;
        p.readParameters("./testParameterFile");
        place pu(p);
        CPPUNIT_ASSERT(std::abs(0.5-pu.getFractionalDecrement())<10*std::numeric_limits<double>::min());
        CPPUNIT_ASSERT(pu.getCleanEveryStep());
        timeStep t(p);
        CPPUNIT_ASSERT(timeStep::timeStepUnit()=="seconds");
        //file asks for timesteps of 3600 seconds
        CPPUNIT_ASSERT(timeStep::deltaT()==timeStep::hour());
        disease d(p);
        CPPUNIT_ASSERT(disease::getRecoveryRate()==0.0008);
        CPPUNIT_ASSERT(disease::getDeathRate()==0.001);
        CPPUNIT_ASSERT(disease::getShed()==0.001);
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class agentTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( agentTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class modelFactoryTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( modelFactoryTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class modelTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( modelTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
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
  runner.addTest( randomTest::suite() );
  runner.addTest( timeStepTest::suite() );
  runner.addTest( travelScheduleTest::suite() );
  runner.addTest( diseaseTest::suite() );
  runner.addTest( placeTest::suite() );
  runner.addTest( parameterTest::suite() );
  runner.addTest( agentTest::suite() );
  runner.addTest( modelFactoryTest::suite() ); 
  runner.addTest( modelTest::suite() ); 
  //run all test suites
  runner.run();
  return 0;
}
