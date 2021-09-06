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
 * @file testing.cpp 
 * @brief Define the progress test listener, and the main function to run all the tests
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
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
#include "../agent.h"
#include<math.h>
#include"randomtest.h"
#include"timereportertest.h"
#include"timesteptest.h"
#include"travelscheduletest.h"
#include"diseasetest.h"
#include"placetest.h"
#include"parametertest.h"
#include"agenttest.h"
#include"modelfactorytest.h"
#include"modeltest.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief Report the start of each test
 *  @details Define a class that will print a message to stdout as each test is started. \n
 *  An instance of this class gets added to the CppUnit::TextUi::TestRunner that runs the test suites
 */
class MyCustomProgressTestListener : public CppUnit::TextTestProgressListener {
 public:
     /** @brief define the message to be printed when each test starts
      *  @details The name of the current running test is extracted using test->getName()
      *  @param test a pointer to a CppUnit::test object
      */
     virtual void startTest(CppUnit::Test *test) {
         std::cout<<"\n starting test "<< test->getName()<<std::endl;
     }
};
/** @brief Run the main set of test suites 
 * @details First set up TestRunner then the Listener to report the start of each test. Then add the listener and each of the test suites to the TestRunner\n
 * Finally call the run method of TestRunner to execute all the test suites
 * */
int main(){
  CppUnit::TextUi::TestRunner runner;
  //add the test reporting
  MyCustomProgressTestListener progress;
  runner.eventManager().addListener(&progress);
  //add test suites
  runner.addTest( randomTest::suite() );
  runner.addTest( timeReporterTest::suite() );
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
/**
 * @mainpage
 * @section intro_sec Introduction
 * This directory contains the unit tests for the mopatop model
 * @subsection Main Main ideas
 * The system uses the cppunit test system, version 1.14 to test each part of the model against some standard  cases\n
 * For each class in the model, a header file is defined here containing tests of that class. Each header containes\n
 * a test suite, each of which runs through some of the methods defined in the classes to check that behave as expected\n
 * in a well defined and limited number of cases where the output can be known ahead of time. This helps to prevent some\n
 * of the more easily caught programming or conceptual errors. Each test suite is added to a testRunner defined in \ref testing.cpp \n
 * Note that because some of the classes can depend on each other, running some of the tests might potentially affect others\n
 * (e.g. the timeStep class is static, so if its parameters are altered, subsequent tests that depend on the timestep might\n
 * be altered). This should be borne in mind when writing new tests.\n
 * When modifications are made to any of the classes, tests should be re-run to chekc that nothing has been broken. Any new\n
 * code not subject to testing should have tests added here as necessary. The tests themsevles have their own documentation\n
 * generated by Doxygen and kept in the html sub-directory.
 * @subsection Compiling Compiling 
 * On a linux system with g++ installed just do \n
 * g++ -otests -std=c++17 -lcppunit ../timestep.cpp ../places.cpp ../disease.cpp testing.cpp\n
 * Note the current version requires g++>= and c++17 in order for the filesystem function to work (for creating new directories etc.)\n
 * @subsection Run Running 
 * At present this is a simple command-line application - just type the executable name (tests above) and then return.\n
 * This will use the parameter file called testParameterFile in this directory. \n
 * \code
 * ./tests
 * \endcode
 * The parameter file allows for a number of tests that depend on parameter settings.
 * @subsection Details
 * A testRunner is defined from the CppUnit::TextUi::TestRunner class. Each of the testsuites is added to this using
 * \code
 * runner.addTest( testClass::suite() );
 * \endcode
 * where testClass is defined in one of the header files. Each header file uses the CPPUNIT_TEST_SUITE macro to define a testsuite\n
 * which is a set of functions, each function containing one or more tests
 * \code
 *   CPPUNIT_TEST_SUITE( testClass );
    //add tests defined below
    CPPUNIT_TEST( function1 );
    CPPUNIT_TEST( function2 );
    CPPUNIT_TEST_SUITE_END();
 * \endcode
 * If needed a setUp() and tearDown() function can be added, which will automatically create and destroy variables afresh at the start of\n
 * each test function e.g.
  * \code
 * public:
    //persistent objects to use during testing
    void setUp()
    {
        p = new place();
    }
    
    void tearDown() 
    {
        delete p;
    }
 * \endcode
 * NB these function defintions must come BEFORE the above testsuite defintions.\n
 * In the test function typically asserts are used to check the result of of a test e.g.
 * \code 
 *  void function1()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        }
 * \endcode
 * These are pass/fail tests, and will be reported  giving the text of the failed test and line number in the code if needed\n
 * In addition tests may report text to stdout where required. Note the uses here of the pre-defined pointer p from the setUp().
 * Variables can also be defined as needed within each test function however.\n
 */
