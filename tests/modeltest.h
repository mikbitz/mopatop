#ifndef MODELTEST_H_INCLUDED
#define MODELTEST_H_INCLUDED

#include"../model.h"
#include<fstream>
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
    /** @brief test the default constructor  */
    CPPUNIT_TEST( testNumberInfected );
    /** @brief test run  */
    CPPUNIT_TEST( testRun );
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure results from constructor are as expected for simple mobile model*/
    void testDefaultConstructor()
    {
        parameterSettings pr;
        //fix the output directory
        pr.setParameter("experiment.run.number","0000");
        model m(pr,"a");
        CPPUNIT_ASSERT(m.numberOfAgents()==600);
        CPPUNIT_ASSERT(m.numberOfPlaces()==280);
        CPPUNIT_ASSERT(m.numberDiseased()==1);
    }
    /** @brief in the simple disease the initial number infected should not be able to exceed the agent number
        @details If more than the number of agents is requested, then the code is expected to max. this out\n
        as the total number of agents initially present*/
    void testNumberInfected()
    {
        parameterSettings pr;
        //check that we can't have more infected than the total number of agents
        pr.setParameter("disease.simplistic.initialNumberInfected","5000");
        model m(pr,"a");
        CPPUNIT_ASSERT(m.numberOfAgents()==600);
        CPPUNIT_ASSERT(m.numberOfPlaces()==280);
        CPPUNIT_ASSERT(m.numberDiseased()==600);
    }
        /** @brief try stepping the model forward and check output
            @details if the number of omp threads is not set this can produce crashes or unpredictable results...*/
    void testRun()
    {
        parameterSettings pr;
        //fix the output directory
        pr.setParameter("experiment.run.number","0000");
        //make sure the thread number is set for open mp!        
        omp_set_num_threads(1);
        model m(pr,"a");

        //take a step
        m.step(0,pr);
        //read header
        std::ifstream f("./output/experiment.default/run_0000/diseaseSummary.csv");
        std::string s;
        std::getline(f,s);
        //check header is OK
        CPPUNIT_ASSERT(s=="step,time(hours),susceptible,infected,recovered,dead");
        std::getline(f,s);
        //this line could change with thread num or RNG settings!
        CPPUNIT_ASSERT(s=="0,0,599,1,0,0");
    }
};

#endif // MODELTEST_H_INCLUDED
