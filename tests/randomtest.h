#ifndef RANDOMTEST_H_INCLUDED
#define RANDOMTEST_H_INCLUDED
#include "../randomizer.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
class randomTest : public CppUnit::TestFixture  {
public:
    //automatically create a test suite to add tests to
    CPPUNIT_TEST_SUITE( randomTest );
    //add tests defined below
    CPPUNIT_TEST( testSeed );
    CPPUNIT_TEST( testDistrib );
    CPPUNIT_TEST_SUITE_END();
    //define tests
    void testSeed()
    {
        //checking same seed gives same sequence
        //note setting seed implies sequence restart
        randomizer r,k;
        CPPUNIT_ASSERT(r.number()==k.number());
        r.setSeed(10);
        CPPUNIT_ASSERT(r.number()!=k.number());
        //k still out of step with r
        k.setSeed(10);
        CPPUNIT_ASSERT(r.number()!=k.number());
        randomizer u(10);
        u.number();//u should now be in step with k
        CPPUNIT_ASSERT(u.number()==k.number());
        //now put both r and k back in step
        k.setSeed(1019377);r.setSeed(1019377);
        CPPUNIT_ASSERT(r.number()==k.number());
    }
    void testDistrib()
    {
        randomizer r(17);
        bool f=true;
        //test strictly between 0 and 1 - expecintg a uniform random distribution
        for (int i=0;i<10000;i++)f=f&&(r.number()<1)&&(r.number()>0);
        CPPUNIT_ASSERT(f);
        //mean should converge on 0.5?? or at least sqrt n fluctuations?
        //the variance should be smaller, the larger the number of samples, fo sure, you'd have thought...but seems not (settles at about 0.083)
        double p=0,q=0,s=0,l=0;
        for (int i=0;i<10;i++)p+=r.number()/10;
        for (int i=0;i<10;i++)s+=pow((r.number()-p),2)/10;
        for (int i=0;i<100000;i++)q+=r.number()/100000;
        for (int i=0;i<100000;i++)l+=pow((r.number()-q),2)/100000;
        CPPUNIT_ASSERT(std::abs(0.5-q)<0.1);
        CPPUNIT_ASSERT(std::abs(0.5-q)<std::abs(0.5-p));
        CPPUNIT_ASSERT(l<s);
        std::cout<<"Mean/Variance of 100,000 RNG samples: "<<q<<" "<<l<<std::endl;
    }

};
#endif // RANDOMTEST_H_INCLUDED
