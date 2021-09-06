#ifndef PARAMETERTEST_H_INCLUDED
#define PARAMETERTEST_H_INCLUDED
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects


class parameterTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( parameterTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testSet );
    CPPUNIT_TEST( testOperator );
    CPPUNIT_TEST( testReadWrite );
    CPPUNIT_TEST( testSetUpObjects );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p.get<int>("run.nSteps")==1);
        CPPUNIT_ASSERT(p.get<long>("run.nAgents")==600);
        CPPUNIT_ASSERT(p.get<int>("run.nThreads")==1);
        CPPUNIT_ASSERT(p.get<int>("run.randomSeed")==0);
        CPPUNIT_ASSERT(p.get("timeStep.units")=="hours");
        CPPUNIT_ASSERT(p.get<double>("timeStep.dt")==1);
        CPPUNIT_ASSERT(p.get("outputFile")=="diseaseSummary.csv");
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
    void testSet()
    {
        parameterSettings p;
        p.setParameter("model.type","test");
        CPPUNIT_ASSERT(p.get("model.type")=="test");
        CPPUNIT_ASSERT(p("model.type")=="test");
        p.setParameter("experiment.run.number","5");
        CPPUNIT_ASSERT(p.get<int>("experiment.run.number")==5);

    }
    void testOperator()
    {
        parameterSettings p;
        CPPUNIT_ASSERT(p("run.nSteps")=="1");
        CPPUNIT_ASSERT(p("experiment.run.number")=="-1");
        CPPUNIT_ASSERT(p("places.cleanContamination")=="false");

    }
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
        disease d(p);
        CPPUNIT_ASSERT(disease::getRecoveryRate()==0.0008);
        CPPUNIT_ASSERT(disease::getDeathRate()==0.001);
        CPPUNIT_ASSERT(disease::getShed()==0.001);
    }
};
#endif // PARAMETERTEST_H_INCLUDED
