#include"../model.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//fixtures allow setting up and tearing down of objects
class modelTest : public CppUnit::TestFixture  {

public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( modelTest );
    //add tests defined below
    CPPUNIT_TEST( testDefaultConstructor );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testDefaultConstructor()
    {
    }
};
