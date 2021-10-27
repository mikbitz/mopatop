#ifndef SCHEDULELISTTEST_H_INCLUDED
#define SCHEDULELISTTEST_H_INCLUDED
#include"../schedulelist.h"
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
 * @file schedulelisttest.h 
 * @brief File containing the definition of the newclassTest class 
 * 
 * @author Mike Bithell
 * @date 27/10/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/** @brief test out the newclass class
    @details what this does */ 
class scheduleListTest : public CppUnit::TestFixture  {

public:
    /** @brief automatically create a test suite to add tests to  */
    CPPUNIT_TEST_SUITE( scheduleListTest );
    /** @brief test the default constructor  */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief check correct types are returned from string arguments */
    CPPUNIT_TEST( testType );
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure results from constructor are as expected 
        @details there should be one mobile and one stationary schedule */
    void testDefaultConstructor()
    {
      scheduleList s;
      unsigned point=s[scheduleList::mobile].getStartPoint();
      CPPUNIT_ASSERT(s[scheduleList::mobile].getCurrentDestination(point)==agent::vehicle);
      point=s[scheduleList::mobile].increment(point);
      CPPUNIT_ASSERT(point==3);
      CPPUNIT_ASSERT(s[scheduleList::mobile].getCurrentDestination(point)==agent::home);
      point=s[scheduleList::stationary].getStartPoint();
      CPPUNIT_ASSERT(s[scheduleList::stationary].getCurrentDestination(point)==agent::home);
    }
    /** @brief check that string names of schedules convert correctly to the scheduleList types 
        @details check "mobile" and "stationary" incorrect names shoudl default to stationary*/
    void testType()
    {
      scheduleList s;
      CPPUNIT_ASSERT(s.getType("mobile")==scheduleList::mobile);
      CPPUNIT_ASSERT(s.getType("stationary")==scheduleList::stationary);
      //unknown type defaults to staionary
      CPPUNIT_ASSERT(s.getType("clonk")==scheduleList::stationary);
    }
};

#endif // NEWCLASSTEST_H_INCLUDED
