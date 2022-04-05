#ifndef MODELFACTORYTEST_H_INCLUDED
#define MODELFACTORYTEST_H_INCLUDED

#include"../modelFactory.h"
/* A program to test the model of modelFactorys moving between places
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
 * @file modelfactorytest.h 
 * @brief File containing the definition of the modelFactoryTest class 
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/** @brief test out the modelFactory class*/ 
class modelFactoryTest : public CppUnit::TestFixture  {

public:
    /** @brief automatically create a test suite to add tests to  */
    CPPUNIT_TEST_SUITE( modelFactoryTest );
    /** @brief test the factory selector method  */
    CPPUNIT_TEST( testSelector );
    /** @brief check agent and place creation  */
    CPPUNIT_TEST( testCreation );
    /** @brief end the test suite   */
    CPPUNIT_TEST_SUITE_END();
    /** @brief make sure the selector works */
    void testSelector()
    {
        modelFactory& F=modelFactorySelector::select("simpleOnePlace");
        simpleOnePlaceFactory H;
        CPPUNIT_ASSERT(typeid(H).name()==typeid(F).name());
        //default model factory in parameterSettings is simpleMobileFactory;
        parameterSettings pr;
        simpleMobileFactory G;
        modelFactory& I=modelFactorySelector::select(pr("model.type"));
        CPPUNIT_ASSERT(typeid(I).name()==typeid(G).name());
    }
    /** @brief check the creation of agents and places looks OK */
    void testCreation()
    {
        std::vector<agent*> agents;
        std::vector<place*> places;
        parameterSettings pr;
        modelFactory& F=modelFactorySelector::select("simpleOnePlace");
        F.createAgents(pr,agents,places,"a");
        CPPUNIT_ASSERT(agents.size()==600);
        CPPUNIT_ASSERT(places.size()==1);
        //agents all in the same place
        for (auto& a:agents)CPPUNIT_ASSERT(a->getHome()==places[0]);
        //work and home are the same
        for (auto& a:agents)CPPUNIT_ASSERT(a->getWork()==places[0]);
        //schedule is just stay home
        for (int i=0;i<100;i++){
            agents[81]->update();timeStep::update();
            CPPUNIT_ASSERT(agents[81]->getHome()==places[0]);
        }
        agents.clear();
        places.clear();
        modelFactory& G=modelFactorySelector::select("simpleMobile");
        //reset date so as to make sure we start at midnight
        timeStep::setDate("Mon 01/01/1900 00:00:00");
        G.createAgents(pr,agents,places,"a");
        CPPUNIT_ASSERT(agents.size()==600);
        CPPUNIT_ASSERT(places.size()==280);
        for (auto& a:agents)CPPUNIT_ASSERT(a->getHome()!=a->getWork());
        for (auto& a:agents)CPPUNIT_ASSERT(a->getHome()==a->getCurrentPlace());
        //schedule is home for from midnight to 0800 hours then transport 1 hour then work, hourly timestep
        for (int i=0;i<17;i++){
            agents[10]->update();timeStep::update();
        }
        CPPUNIT_ASSERT(agents[10]->getWork()==agents[10]->getCurrentPlace());
    }
};

#endif // MODELFACTORYTEST_H_INCLUDED
