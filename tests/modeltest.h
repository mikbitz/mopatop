#ifndef MODELTEST_H_INCLUDED
#define MODELTEST_H_INCLUDED

#include"../model.h"
/* A program to test the model of models moving between places
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
 * @file modeltest.h 
 * @brief File containing the definition of the modelTest class 
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/** @brief test out the model class*/ 
class modelTest : public CppUnit::TestFixture  {

public:
    /** @brief automatically create a test suite to add tests to  */
    CPPUNIT_TEST_SUITE( modelTest );
    /** @brief test the default constructor  */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure results from constructor are as expected for simple mobile model*/
    void testDefaultConstructor()
    {
        parameterSettings pr;
        model m(pr);
        CPPUNIT_ASSERT(m.numberOfAgents()==600);
        CPPUNIT_ASSERT(m.numberOfPlaces()==280);
        CPPUNIT_ASSERT(m.numberDiseased()==1);
    }
};

#endif // MODELTEST_H_INCLUDED
