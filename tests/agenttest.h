#ifndef AGENTTEST_H_INCLUDED
#define AGENTTEST_H_INCLUDED

#include"../agent.h"
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
 * @file agenttest.h 
 * @brief File containing the definition of the agentTest class 
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/** @brief test out the agent class*/ 
class agentTest : public CppUnit::TestFixture  {

public:
    /** @brief automatically create a test suite to add tests to  */
    CPPUNIT_TEST_SUITE( agentTest );
    /** @brief test the default constructor  */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief test some of the set/get functions  */
    CPPUNIT_TEST( testSettings );
    /** @brief test the disease functions  */
    CPPUNIT_TEST( testDisease );
    /** @brief test the schedule  */
    CPPUNIT_TEST( testSchedule );
    /** @brief test occupancy lists  */
    CPPUNIT_TEST( testMove );
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure default agent has no disease, is alive, and knows about 3 kinds of place. Check ID increments.
        @details Note that since the agent ID auto-increments, this has to be the first test suite that uses any agents\n
        if the first agent tested is meant to have ID zero */
    void testDefaultConstructor()
    {
        agent a,b;
        CPPUNIT_ASSERT(a.getID()==0);
        CPPUNIT_ASSERT(b.getID()==1);
        CPPUNIT_ASSERT(!a.diseased());
        CPPUNIT_ASSERT(!a.recovered());
        CPPUNIT_ASSERT(!a.immune());
        CPPUNIT_ASSERT(a.alive());
        CPPUNIT_ASSERT(a.places.size()==3);
    }
    /** @brief test the ID and places can be set */
    void testSettings()
    {   
        agent a;
        place p;
        CPPUNIT_ASSERT(a.getID()==2);
        a.setID(30);
        CPPUNIT_ASSERT(a.getID()==30);
        a.setHome(&p);
        CPPUNIT_ASSERT(a.getHome()==&p);
        //setHome also sets current place to home
        CPPUNIT_ASSERT(a.getCurrentPlace()==&p);
        a.setWork(&p);
        CPPUNIT_ASSERT(a.getWork()==&p);
        a.setTransport(&p);
        CPPUNIT_ASSERT(a.getTransport()==&p);        
        parameterSettings pr;
        a.initTravelSchedule(pr);
    }
    /** @brief test the travel schedule */
    void testSchedule()
    {   
        agent a;
        place h,w,t;   
        parameterSettings pr;
        //default parameter settings have simple mobile schedule
        a.initTravelSchedule(pr);
        //set the places
        a.setHome(&h);
        a.setWork(&w);
        a.setTransport(&t);
        //should start on transport, but initTravelSchedule immediately moves to next destination, which is home
        CPPUNIT_ASSERT(a.getCurrentPlace()==&h);
        //should stay at home for a while - 14 hours
        a.update();
        CPPUNIT_ASSERT(a.getCurrentPlace()==&h);
        for (int i=0;i<13;i++)a.update();
        CPPUNIT_ASSERT(a.getCurrentPlace()==&t);
        a.update();
        //now at work - for 8 hours
        CPPUNIT_ASSERT(a.getCurrentPlace()==&w);
        for (int i=0;i<8;i++)a.update();
        CPPUNIT_ASSERT(a.getCurrentPlace()==&t);
        a.update();
        CPPUNIT_ASSERT(a.getCurrentPlace()==&h);
        
    }
    /** @brief test the function that changes occupancy lists of places */
    void testMove()
    {   
        agent a;
        place h,w; 
        a.setHome(&h);
        a.setWork(&w);
        a.moveTo(agent::home);
        CPPUNIT_ASSERT(h.getNumberOfOccupants()==1);
        CPPUNIT_ASSERT(a.getCurrentPlace()==&h);
        a.moveTo(agent::work);
        CPPUNIT_ASSERT(h.getNumberOfOccupants()==0);
        CPPUNIT_ASSERT(w.getNumberOfOccupants()==1);
        CPPUNIT_ASSERT(a.getCurrentPlace()==&w);
    }
    /** @brief Check the functions that set the disease are working as expected */
    void testDisease()
    {
        agent a,b,c,d;
        place p;
        a.setHome(&p);
        CPPUNIT_ASSERT(!a.diseased());
        a.becomeInfected();
        CPPUNIT_ASSERT(a.diseased());
        CPPUNIT_ASSERT(!a.recovered());
        CPPUNIT_ASSERT(!a.immune());
        a.recover();
        CPPUNIT_ASSERT(!a.diseased());
        CPPUNIT_ASSERT(a.recovered());
        CPPUNIT_ASSERT(a.immune());
        CPPUNIT_ASSERT(a.alive());
        a.die();
        CPPUNIT_ASSERT(!a.diseased());
        CPPUNIT_ASSERT(!a.recovered());
        CPPUNIT_ASSERT(!a.immune());
        CPPUNIT_ASSERT(!a.alive());
        CPPUNIT_ASSERT(!a.alive());
        b.setHome(&p);
        b.becomeInfected();
        //should contaminate the current place
        b.cough();
        CPPUNIT_ASSERT(b.getCurrentPlace()->getContaminationLevel()==disease::getShed());
        b.recover();
        //contamination should be unchanged as agent has recovered
        CPPUNIT_ASSERT(b.getCurrentPlace()->getContaminationLevel()==disease::getShed());
        //put contamination to over 1 - guarantees infectious!
        b.getCurrentPlace()->increaseContamination(1);
        randomizer r;
        //b should be immune
        b.process_disease(r);
        CPPUNIT_ASSERT(!b.diseased());
        //a is dead!
        a.process_disease(r);
        CPPUNIT_ASSERT(!a.diseased());
        //new agent should get infected
        c.setHome(&p);
        c.process_disease(r);
        CPPUNIT_ASSERT(c.diseased());
        //store disease default recovery rate- needed for later tests
        double k=disease::getRecoveryRate();
        disease::setRecoveryRate(1.);
        //c should recover!
        c.process_disease(r);
        CPPUNIT_ASSERT(c.recovered());
        //reset disease
        disease::setRecoveryRate(k);
        //check for death!
        k=disease::getDeathRate();
        disease::setDeathRate(1.);
        d.setHome(&p);
        d.becomeInfected();
        d.process_disease(r);
        CPPUNIT_ASSERT(!d.alive());
        disease::setDeathRate(k);
    }
};

#endif // AGENTTEST_H_INCLUDED
