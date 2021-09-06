#ifndef DISEASETEST_H_INCLUDED
#define DISEASETEST_H_INCLUDED
#include "../disease.h"
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
#endif // DISEASETEST_H_INCLUDED
