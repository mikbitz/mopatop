#ifndef DISEASETEST_H_INCLUDED
#define DISEASETEST_H_INCLUDED
#include "../disease.h"
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
 * @file diseasetest.h 
 * @brief File containing the definition of the diseaseTest class 
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief test the disease static classs
 *  @details */
class diseaseTest : public CppUnit::TestFixture  {
public:
    /** @brief automatically create a test suite to add tests to */
    CPPUNIT_TEST_SUITE( diseaseTest );
    /** @brief check defaults */
    CPPUNIT_TEST( testDefaults );
    /** @brief test out constructor */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief test recovery from disease */
    CPPUNIT_TEST( testRecover );
    /** @brief test out the death function */
    CPPUNIT_TEST( testDie );
    /** @brief check how infection is acquired */
    CPPUNIT_TEST( testInfection );
    /** @brief test contamination shedding */
    CPPUNIT_TEST( testShed );
    /** @brief end test suite */
    CPPUNIT_TEST_SUITE_END();
    /** @brief since this is a static class defaults should be as in disease.cpp */
    void testDefaults()
    {
        CPPUNIT_ASSERT(0.0004==disease::getRecoveryRate());
        CPPUNIT_ASSERT(0.1==disease::getDeathRate());
        CPPUNIT_ASSERT(0.002==disease::getShed());
    }
    /** @brief by defining an object values are set by teh constructor rater than defaults */ 
    void testDefaultConstructor()
    {
        disease d;
        CPPUNIT_ASSERT(0.0008==disease::getRecoveryRate());
        CPPUNIT_ASSERT(0.000==disease::getDeathRate());
        CPPUNIT_ASSERT(0.001==disease::getShed());
    }
    /** @brief given two identical  random sequences, recovery should be the same
     @details explcitly check that the function call to disease matches the function content. Also make \n
     sure that the timestepping guarantees the same rate per unit time independentof time unit */ 
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
    /** @brief the death function looks just like the recovery function so test in the same way */ 
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
    /** @brief again use two identical random sequences to check infection
        @details this should just depend on the contamination level and the rate per hour. */
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
    /** @brief Check infection shedding is independent of timestep unit also */
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
