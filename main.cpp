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
#include"parameters.h"
#include"timereporter.h"
#include"randomizer.h"
#include"randomizerSingleton.h"
#include"agent.h"
#include "model.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//default variables from static classes - these are here to keep the linker happy
//should really be in the header files, but at the moment the linker complains about multiple definitions


//------------------------------------------------------------------------
//------------------------------------------------------------------------
/** set up and run the model 
 @param argc The number of command line arguments - at the moment only 1 can be handled
 @param argv The argument values - expected to be just the name of the parameter file */
int main(int argc, char **argv) {

    std::string name="Mopatop";
    std::string version="0.4";
    std::cout<<name<<" model version "<<version<<std::endl;

    //work out the current local time using C++ clunky time 
    std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    std::cout<<"Run set started at: "<<ctime(&t)<<std::endl;
    //time the whole set of runs
    auto startSet=timeReporter::getTime();
    //set up the parameters using an optional command-line argument
    //first set defaults
    parameterSettings parameters;
    parameters.setParameter("model.version",version);
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
        if(runs>0)parameters.setParameter("experiment.run.number","-1");
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



