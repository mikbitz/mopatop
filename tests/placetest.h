#ifndef PLACETEST_H_INCLUDED
#define PLACETEST_H_INCLUDED

#include"../places.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class placeTest : public CppUnit::TestFixture  {
    /** @brief A pointer to a default place for the setUp method */
    place* p;
public:
    /** @brief set up a default place pointer */
    void setUp()
    {
        p = new place();
    }
    /** @brief delete the place */
    void tearDown() 
    {
        delete p;
    }
    /** @brief automatically create a test suite to add tests to - note this has to come after any setup/tearDown */
    CPPUNIT_TEST_SUITE( placeTest );
    /** @brief check constructor works */
    CPPUNIT_TEST( testDefaultConstructor );
    /** @brief check place ID */
    CPPUNIT_TEST( testID );
    /** @brief test contamination */
    CPPUNIT_TEST( testContamination );
    /** @brief contamination should decay*/
    CPPUNIT_TEST( testAgents );
    /** @brief agents should be added and removed from place correctly */
    CPPUNIT_TEST_SUITE_END();
    /** @brief The constructor should set the necessary default values 
     *  @details use std::numeric_limits<double>::min() here to find the smallest possible double\n
     *  so that test will not fail as a result of rounding errors. */
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
    /** @brief Check the deafult ID is zero and that it can be set with setID */
    void testID()
    {
        //default ID is zero
        CPPUNIT_ASSERT(0==p->getID());
        p->setID(73);
        CPPUNIT_ASSERT(73==p->getID());
    }
    /** @brief Check the contamination levels
     * @details values should increase as expected, not be able to go below zero and get reset to zero by cleanContamination() 
     **/ 
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
    /** @brief Check the update exponentially decreases contamination 
     *  @details It shoudl aslo be possible to set na diunset the cleanContamination flag that makes contamination
     *  go to zero after every step (or not, whne it is unset).
     * */
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
    /** @brief check occupancy
     *  @details it should only be possible to add an agent once. removal should not crash if non-resident agent is removed.
     *  number of agents should increase and decrease correctly. The show function shoudl correctly list IDs of residents. 
     */
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

#endif // PLACETEST_H_INCLUDED
