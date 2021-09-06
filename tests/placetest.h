#include"../places.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class placeTest : public CppUnit::TestFixture  {
    place* p;
public:
    //persistent objects to use during testing
    void setUp()
    {
        p = new place();
    }
    
    void tearDown() 
    {
        delete p;
    }
    /** @brief automatically create a test suite to add tests to - note this has to come after any setup/tearDown */
    CPPUNIT_TEST_SUITE( placeTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST( testID );
    CPPUNIT_TEST( testContamination );
    CPPUNIT_TEST( testUpdate );
    CPPUNIT_TEST( testAgents );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        CPPUNIT_ASSERT(0==p->getNumberOfOccupants());
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        CPPUNIT_ASSERT(std::abs(0.1-p->getFractionalDecrement())<10*std::numeric_limits<double>::min());
        CPPUNIT_ASSERT(!p->getCleanEveryStep());
        p->setCleanEveryStep();
        CPPUNIT_ASSERT(p->getCleanEveryStep());
        p->unsetCleanEveryStep();
        CPPUNIT_ASSERT(!p->getCleanEveryStep());
    }
    void testID()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        p->setID(73);
        CPPUNIT_ASSERT(73==p->getID());
    }
    void testContamination()
    {
        p->increaseContamination(0.1);
        //make sure value is close to 0.1 - because we are using doubles, it might not be exaclty equal
        CPPUNIT_ASSERT(std::abs(0.1-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->increaseContamination(0.1);
        CPPUNIT_ASSERT(std::abs(0.2-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->cleanContamination();
        //should be nearly zero
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //negatives get reset to zero.
        p->increaseContamination(-0.1);
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
    }
    void testUpdate()
    {
        p->increaseContamination(0.1);
        //make sure value is close to eseentially zero - because we are using doubles, it might not be exaclty equal
        p->setCleanEveryStep();
        p->update();
        CPPUNIT_ASSERT(std::abs(p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->unsetCleanEveryStep();
        p->increaseContamination(0.1);
        p->setFractionalDecrement(0.0);
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        p->setFractionalDecrement(0.5);
        //timestep is 1 hour by default.
        timeStep::setdeltaT(timeStep::hour());
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //set timestep to half an hour
        timeStep::setdeltaT(0.5*timeStep::hour());
        p->update();
        CPPUNIT_ASSERT(std::abs(0.1*exp(-0.5)*exp(-0.5*0.5)-p->getContaminationLevel())<10*std::numeric_limits<double>::min());
        //change back to default just in case of later uses
        timeStep::setdeltaT(timeStep::hour());
    }
    void testAgents()
    {
        agent* a=new agent();
        p->add(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->add(a);
        //agents are unique
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        //safe to remove non existent agent?
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        agent* b=new agent();
        p->remove(b);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==0);
        //now multiple (non-unique!) adds
        p->add(a);
        p->add(a);
        p->add(b);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==2);
        p->remove(a);
        CPPUNIT_ASSERT(p->getNumberOfOccupants()==1);
        p->add(a);
        a->setID(10);
        b->setID(15);
        //test the method to show number of occupants (default no listing)
        p->show();
        //list occupant IDs
        p->show(true);
        
    }
};
