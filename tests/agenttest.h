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
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure default agent has no disease, is alive, and knows about 3 kinds of place. Check ID increments.
        @details Note that since the agent ID auto-increments, this has to be the first test suite if the first agent tested
        is meant to have ID zero */
    void testDefaultConstructor()
    {
        agent a,b;
        CPPUNIT_ASSERT(a.getID()==0);
        CPPUNIT_ASSERT(b.getID()==1);
        CPPUNIT_ASSERT(a.diseased()==false);
        CPPUNIT_ASSERT(a.recovered()==false);
        CPPUNIT_ASSERT(a.immune()==false);
        CPPUNIT_ASSERT(a.alive()==true);
        CPPUNIT_ASSERT(a.places.size()==3);
    }
};

#endif // AGENTTEST_H_INCLUDED
