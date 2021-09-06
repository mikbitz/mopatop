#ifndef MODELFACTORYTEST_H_INCLUDED
#define MODELFACTORYTEST_H_INCLUDED
#include"../modelFactory.h"
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
#endif // MODELFACTORYTEST_H_INCLUDED
