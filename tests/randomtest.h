#ifndef RANDOMTEST_H_INCLUDED
#define RANDOMTEST_H_INCLUDED
#include "../randomizer.h"
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
 * @file randomtest.h 
 * @brief File containing the definition of the randomTest class for the random number wrapper
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** @brief Test some of the random number generator behaviour
 *  @details Check the random seed setting works, and try to see whether the distribution of random numbers
 *  seems sensible.*/
class randomTest : public CppUnit::TestFixture  {
public:
    /** @brief automatically create a test suite */
    CPPUNIT_TEST_SUITE( randomTest );
    /** @brief seed test */
    CPPUNIT_TEST( testSeed );
    /** @brief distribution test */
    CPPUNIT_TEST( testDistrib );
    /** @brief end test suite */
    CPPUNIT_TEST_SUITE_END();
    /** @brief test random seed behaviour
     *  @details It is expected that the same random sequence is generated by the same seed. When the seed is reset\n
     *  using the setSeed method, the random sequence should restart. Each call to the number() method should advance\n
     *  the sequence by one step. The constructor should default to seed 0, but allow a seed to be set as an argument.
     */
    void testSeed()
    {
        //checking same seed gives same sequence (zero seed)
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
    /** @brief Run through a large number of randoms to see if teh sequence seems uniform-ish\n
     * @details The sequence should lie strictly in (0,1). The mean should converge on 0.5\n
     * For large number, one might expect the variance about the mean to decrease steadily as the number\n
     * of invocations increases, but instead it steadies at about 0.083 - a little consideration reveals\n
     * that the expected variance should indeed converge on 1/12 for a square distribtuion of width 1.
     */
    void testDistrib()
    {
        randomizer r(17);
        bool f=true;
        //test strictly between 0 and 1 - expecintg a uniform random distribution
        for (int i=0;i<10000;i++)f=f&&(r.number()<1)&&(r.number()>0);
        CPPUNIT_ASSERT(f);
        //mean should converge on 0.5 or at least sqrt n fluctuations - although the behaviour at small n is rather uneven.
        //the variance should be smaller, the larger the number of samples, fo sure, you'd have thought...but settles at about 0.083, as expected
        double p=0,q=0,s=0,l=0;
        for (int i=0;i<10;i++)p+=r.number()/10;
        for (int i=0;i<10;i++)s+=pow((r.number()-p),2)/10;
        for (int i=0;i<100000;i++)q+=r.number()/100000;
        for (int i=0;i<100000;i++)l+=pow((r.number()-q),2)/100000;
        CPPUNIT_ASSERT(std::abs(0.5-q)<0.1);
        CPPUNIT_ASSERT(std::abs(0.5-q)<std::abs(0.5-p));
        CPPUNIT_ASSERT(l<s);
        CPPUNIT_ASSERT(abs(1./12-l)<0.001);
    }

};
#endif // RANDOMTEST_H_INCLUDED
