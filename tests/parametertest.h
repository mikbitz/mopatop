#ifndef PARAMETERTEST_H_INCLUDED
#define PARAMETERTEST_H_INCLUDED
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
 * @file parametertest.h 
 * @brief File containing the definition of the parameterTest class for testing the parameter file
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
/** @brief Check parameter defaults are as expected
 *  @details Setting of parameters and ability to read from and write out to a file are checked. \n
 *  The input file is called testParameterFile, and the output RunParameters.\n
 *  Text showing parameter settings is also displayed on stdout.
 */
class parameterTest : public CppUnit::TestFixture  {

public:
    /** @brief automatically create a test suite to add tests to  */
    CPPUNIT_TEST_SUITE( parameterTest );
    /** @brief constructor shoudl set defaults for all parameters */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief parameters should be settable */
    CPPUNIT_TEST( testSet );
    /** @brief operator () returns string value */
    CPPUNIT_TEST( testOperator );
    /** @brief read and write parameter settings */
    CPPUNIT_TEST( testReadWrite );
    /** @brief Test settings in other objects */
    CPPUNIT_TEST( testSetUpObjects );
    /** @brief End test suite */
    CPPUNIT_TEST_SUITE_END();
    /** @brief The default constructor calls the setdefaults function
        @details The values here are copied directly from that function. Also tests that the get<> methods\n
        correctly retrieve parmeters of the right datatype */
    void testDefaultConstructor()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p.get<int>("run.nSteps")==1);
        CPPUNIT_ASSERT(p.get<long>("run.nAgents")==600);
        CPPUNIT_ASSERT(p.get<int>("run.nThreads")==1);
        CPPUNIT_ASSERT(p.get<int>("run.randomSeed")==0);
        CPPUNIT_ASSERT(p.get("timeStep.units")=="hours");
        CPPUNIT_ASSERT(p.get<double>("timeStep.dt")==1);
        CPPUNIT_ASSERT(p.get("timeStep.startdate")=="Mon 01/01/1900 00:00:00");
        CPPUNIT_ASSERT(p.get("outputFile")=="diseaseSummary");
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
    /** @brief Check settings of individual parameters works
     *   @details make sure that the string values in set get retrieved as correct data values */
    void testSet()
    {
        parameterSettings p;
        p.setParameter("model.type","test");
        CPPUNIT_ASSERT(p.get("model.type")=="test");
        CPPUNIT_ASSERT(p("model.type")=="test");
        p.setParameter("experiment.run.number","5");
        CPPUNIT_ASSERT(p.get<int>("experiment.run.number")==5);

    }
    /** @brief The operator () should return the string value corresponding to any parameter
     *   @details check strings are as expected from defaults */
    void testOperator()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p("run.nSteps")=="1");
        CPPUNIT_ASSERT(p("experiment.run.number")=="-1");
        CPPUNIT_ASSERT(p("places.cleanContamination")=="false");

    }
    /** @brief Check input and output of parameter settings
     *   @details The default output file is called RunParameters - here just the path is set to the current directory.\n
     *    A few of the values from the input file testParameterFile are checked where they differ from defaults */
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
        /** @brief Check some of the other classes are getting the right input from the parameterSettings objects
     *   @details In this case the static timeStep and disease, as well as the places */
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
        //start date is set to 23/02/2022, but model assumes 0 for first day of month, and first month
        CPPUNIT_ASSERT(timeStep::getDayOfMonth()==22);
        CPPUNIT_ASSERT(timeStep::getMonth()==1);
        CPPUNIT_ASSERT(timeStep::getYear()==2022);
        disease d(p);
        CPPUNIT_ASSERT(disease::getRecoveryRate()==0.0008);
        CPPUNIT_ASSERT(disease::getDeathRate()==0.001);
        CPPUNIT_ASSERT(disease::getShed()==0.001);
    }
};
#endif // PARAMETERTEST_H_INCLUDED
