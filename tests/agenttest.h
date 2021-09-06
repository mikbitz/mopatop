#include"../agent.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class agentTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( agentTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class modelFactoryTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( modelFactoryTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
