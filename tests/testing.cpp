#include <iostream>
#include <string>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TextTestProgressListener.h>
#include "../places.h"
#include<math.h>

class placeTest : public CppUnit::TestFixture  {
    place* p;
public:
    void setUp()
    {
        p = new place();
    }
    
    void tearDown() 
    {
        delete p;
    }
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( placeTest );
    //add tests defined below
    CPPUNIT_TEST( testID );
    CPPUNIT_TEST( testContamination );
    CPPUNIT_TEST_SUITE_END();
    void testID()
    {
        p->setID(1);
        CPPUNIT_ASSERT(1==p->getID());
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
};

//report the start of each test
class MyCustomProgressTestListener : public CppUnit::TextTestProgressListener {
 public:
     virtual void startTest(CppUnit::Test *test) {
         std::cout<<"\n starting test "<< test->getName()<<std::endl;
     }
};
int main(){
  CppUnit::TextUi::TestRunner runner;
  //add the test reporting
  MyCustomProgressTestListener progress;
  runner.eventManager().addListener(&progress);
  //add test suites
  runner.addTest( placeTest::suite() );
  //run all test suites
  runner.run();
  return 0;
;
}
