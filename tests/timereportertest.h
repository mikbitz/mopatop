#ifndef TIMEREPORTERTEST_H_INCLUDED
#define TIMEREPORTERTEST_H_INCLUDED
#include"../timereporter.h"
#include<unistd.h>
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
 * @file timereportertest.h 
 * @brief File containing the definition of the timeReporterTest class for reporting run-time intervals
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief Check intervals are computed and reported as expected
 */
class timeReporterTest : public CppUnit::TestFixture  {
public:
    /** @brief automatically create a test suite */
    CPPUNIT_TEST_SUITE( timeReporterTest );
    /** @brief check that intervals are measured correctly */
    CPPUNIT_TEST( testInterval);
    /** @brief end test suite */
    CPPUNIT_TEST_SUITE_END();
    /** @brief Check that wating for a few milliseconds is correctly reported
     * @details uses the usleep function to wait a specified number of microseconds between two calls to timeReporter::getTime \n
     * Tests the timereporter interval function to be sure it is always positive.Then outputs some text for the user to check \n
     * that the reporting is as expected (since showInterval should write to stdout)
     */
    void testInterval()
    {
        auto start=timeReporter::getTime();
        //wait 10,000 microseconds
        usleep(10000);
        auto end=timeReporter::getTime();
        CPPUNIT_ASSERT(timeReporter::interval(end,start)==10);
        CPPUNIT_ASSERT(timeReporter::interval(start,end)==10);
        timeReporter::showInterval("This should output the text: 0.01 seconds :",start,end);
    }

    
};
#endif // TIMEREPORTERTEST_H_INCLUDED
