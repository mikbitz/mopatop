/* A program to model agents moveing between places
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
/**
 * @file main.cpp 
 * @brief File containing some classes and method definitions and the main function
 * 
 * @author Mike Bithell
 * @date 10/08/2021
 **/
#include<iostream>
#include<algorithm>
#include<map>
#include<fstream>
#include<vector>
#include<set>
#include<string>
#include<assert.h>
#include<omp.h>
#include<filesystem>
#include"parameters.h"
#include"timereporter.h"
#include"randomizer.h"
#include"disease.h"
#include"places.h"
#include"timestep.h"
#include"agent.h"
#include"travelschedule.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//needed here for places as agents are pre-declared before place class
/** @param listAll Optional argument to show every place - only useful if there are just a few! */
void place::show(bool listAll=false){
    std::cout<<"Place ID "<<ID<<" has "<<occupants.size()<<" occupants"<<std::endl;
    if (listAll){
        std::cout<<"List of Occupant IDs:- "<<std::endl;
        for (auto o : occupants){
            std::cout <<o->ID<<std::endl;
        }
    }
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//defined here so as to be after travelSchedule
void agent::update()
{
        //Use the base travel schedule - initialised at home for everyone
        counter-=timeStep::deltaT();//reduce by actual time represented by this timestep (since schedule is defined in hours rater than timesteps)
        if (counter<=0){
            currentPlace=schedule->getNextLocation();
            counter=schedule->getTimeAtCurrentPlace();
        }
        //expensive - only needed if agents need direct agent-to-agent interactions in a place -
        //might be made cheaper by alowing agents to be present in multiple places, but only active in one.
        //(this could allow for remote meetings/phone calls?!)
        //moveTo(currentPlace);
        if (currentPlace==home)atHome();//people might be at some other location overnight - e.g. holiday, or trucker in their cab - but home can have special properties (e.g. food storage, places where I keep my stuff)
        if (currentPlace==vehicle)inTransit();
        if (currentPlace==work)atWork();//this could involve travelling too - e.g. if delivery driver 
        
}
void agent::initTravelSchedule(){       
   schedule=new  travelSchedule();
   currentPlace=schedule->getNextLocation();
   counter=schedule->getTimeAtCurrentPlace();
}
void agent::cough()
{
        //breathInto(place) - scales linearly with the time spent there (using uniform timesteps) - masks could go here as a scaling on contamination increase (what about surfaces? -second contamination factor?)
        if (diseased) places[currentPlace]->increaseContamination(disease::shedInfection());
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//include the model header file here so it knows the definitions of agent/place etc.
#include "model.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** set up and run the model 
 @param argc The number of command line arguments - at the moment only 1 can be handled
 @param argv The argument values - expected to be just the name of the parameter file */
int main(int argc, char **argv) {


    std::cout<<"Model version 0.2"<<std::endl;

    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set started at: "<<ctime(&t)<<std::endl;
    //time the whole set of runs
    auto startSet=timeReporter::getTime();
    //set up the parameters using an optional command-line argument
    //first set defaults
    parameterSettings parameters;
    parameters.setParameter("model.version","v0.2");
    //now read from a file
    if (argc ==1){
        std::cout<<"Using default parameter file"<<std::endl;
        parameters.readParameters("../defaultParameterFile");
    }else{
        std::cout<<"Default parameter file overridden on command line"<<std::endl;
        parameters.readParameters(argv[1]);
    }
    //set the number of OMP threads used to parallelise loops
    omp_set_num_threads(parameters.get<int>("run.nThreads"));

    //make sure there is at least one run!
    if (parameters.get<int>("run.nRepeats")<=0){
        parameters.setParameter("run.nRepeats","1");
    }
    //initialise the disease - since this is a static class, this just need to be done for a single instance
    disease d(parameters);
    //repeat the model run nRepeats times with different random seeds
    int seed=parameters.get<int>("run.randomSeed");
    int increment=parameters.get<int>("run.randomIncrement");
    
    for (int runs=0;runs<parameters.get<int>("run.nRepeats");runs++){
        std::time_t tr=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        std::cout<<"Run repeat number: "<<runs+1<<" started at"<<ctime(&tr)<<std::endl;
        //increment the random seed value - note intially runs=0, so the default seed gets used
        parameters.setParameter("run.randomSeed",std::to_string(seed+runs*increment));
        //If number of runs>1, clear out the run number so that it will get set to auto-increment on each run.
        //This is needed to get the directory structure right - see model::setOutputFilePaths
        if(runs>0)parameters.setParameter("experiment.run.number","");
        //create and initialise a new model
        //any variations to parameter values should happen before this, so that the values
        //get properly saved to the output RunParameters file (created by model::setOutputFilePaths)
        model m(parameters);
        //start a timer to record the execution time
        auto start=timeReporter::getTime();
        //loop over time steps
        for (int step=0;step<parameters.get<int>("run.nSteps");step++){
            if (step%100==0)std::cout<<"Start of step "<<step<<std::endl;
            m.step(step,parameters);
        }
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Execution time after initialisation: ",start,end);
        t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        std::cout<<"Run finished at: "<<ctime(&tr)<<std::endl;
    }
    //report overall time taken
    auto endSet=timeReporter::getTime();
    timeReporter::showInterval("Total execution time for all runs: ",startSet,endSet);
    //work out the finish time
    t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set finished at: "<<ctime(&t)<<std::endl;
    return 0;
}



