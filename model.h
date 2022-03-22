#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED
#include<filesystem>
#include<omp.h>
/* A program to model agents moving between places
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
 * @file model.h 
 * @brief File containing the definition \ref model class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief The model contains all the agents and places, and steps them through time
 * @details At the moment time steps are not even in length (since the schedule values are not of the same length for work/home/transport)\n

*/
#include"modelFactory.h"
#include"timereporter.h"
#ifdef COUPLER
#include "fetchall.h"
#endif
class model{
    /** @brief A container to hold pointers to all the locally resident agents\n
        @details Here locally resident means that all these agents are on the current MPI domain.*/
    std::vector<agent*> agents;
    /** @brief A container to hold pointers to agents from another MPI domain.
       @details These agents are only present if they have travelled from another copy of the model running on a different (HPC) node\n
                  see \ref coupler below*/
    std::vector<agent*> travellers;
    /** @brief A container to hold the local places */
    std::vector<place*> places;
    /** @brief The number of agents to be created */
    long nAgents;
    /** @brief A string containing the file path for output, for a given experiment, to be put before specific file names */
    std::string _filePrefix;
    /** @brief A string containing any extra default characters to come after the filename */
    std::string _filePostfix;
    /** @brief The output file stream */
    std::ofstream output;
    /** @brief variable to hold the random number generator for this model
        @details This is currently directly created with default seed 0, rather than being a singleton (accessible from anywhere in the code)\n
        see \ref randomizerSingleton.h for singleton code. At the moment a singleton is not used as it seems a little tricky in multi-threaded cases\n
        as a result model randomizers have to be passed into client functions*/
    randomizer r;
    /** @brief multi threaded runs are faster if each thread has its own RNG
      @details Each RNG is created with its own seed, seeds being separated by 1 incrementally. This means that  multi-threaded runs\n
      can be reproduced if the same number of threads is used (and the same start random seed), but runs with different numbers of threads\n
      will typically produce different output (to the extent that output is stochastic)*/
    std::vector<randomizer> randoms;
#ifdef COUPLER
    /** @brief A pointer to the model coupler, if required.
        @details This allows for various different models to be coupled together with MPI as specified by \ref fetchall.h \n
        It can also be used to split this model over several MPI domains, where each domain runs one copy of the model and agents get\n
        transferred between domains if needed - useful if the model gets large enough that it won't fit in a single (high performance computer) node. \n
        For example, one might divide the world into two sections (e.g. North and southern hemisphere) - agents then have to cross domain if they travel \n
        from NH to SH.*/
    MUIcoupler* coupler;
#endif
    /** @brief The name of the MPI domain for use with MUI and this copy of the model */ 
    std::string domain;
    /** @brief Flag if agents leaving the domain in this step */
    bool leavers=false;
public:
    /** @brief Constructor for the model - set up the random seed and the output file, then call \ref init to define the agents and the places \n
        @details The time reporter class is used to check how long it takes to set up everything. The static timestep class is initialised from the parameter file \n
        The model allows for there to be mutiple copies to be running simultaneously (possibly coupled together)\n
        The domain string is a label unique to each running copy. These can be coupled using the MUI coupler \n
        and MPI - the idea being that a model can be split up across a cluster to allow for very large models \n
        or to allow the model to be coupled to another kind of model (e.g. an ecosystem model). See \ref fetchall.h \n
        If this is not needed then the domain is ignored.
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied
        @param domain A string that defines which MPI domain this is, when using domain decomposition to run multiple models using MPI - see fetchall.h */
    model(parameterSettings& parameters,std::string dom):domain(dom){
         //If using the MUI coupler, initialise the domain
#ifdef COUPLER

        coupler=new MUIcoupler(domain);

#endif
        leavers=false;
        //timestep is a static class - need only set its parameters once, here.
        timeStep t(parameters);
        nAgents=parameters.get<long>("run.nAgents");
        //create mutiple RNG for many threaded runs
        for (int i=0;i<parameters.get<int>("run.nThreads");i++){
            randomizer r(parameters.get<int>("run.randomSeed")+i);
            randoms.push_back(r);
        }

        //create the directories and paths for the current experiment
        setOutputFilePaths(parameters);
        //output file
        output.open(_filePrefix+parameters("outputFile")+_filePostfix+".csv");
        //header line
        output<<"step,time(hours),susceptible,infected,recovered,dead"<<std::endl;
        //Initialisation can be slow - check the timing
        auto start=timeReporter::getTime();
        init(parameters,domain);
        auto end=timeReporter::getTime();
        timeReporter::showInterval("Initialisation took: ", start,end);
    }
    //------------------------------------------------------------------------
    /** @brief destructor - make sure output files are properly closed */
    ~model(){
        output.close();
        randoms.clear();
    }
    //------------------------------------------------------------------------
    /** @brief Create the system of directories for model experiments and their outputs
     *  @details A system of directories gets created, starting from the top level experiment.output.directory. \n
     *  To this is added the name of an experiment, which is assumed to consist of a number of separate runs with \n
     *  different parameter values (e.g. random seed). By default a max. of 10000 runs is assumed - this gives tidy names\n
     *  like ./output/experiment.test/run_0000 ./output/experiment.test/run_0001 ... ./output/experiment.test/run_9999 \n
     *  More then 10000 will be ok, but the directory names will spill over to ./output/experiment.test/run_10000 etc. \n
     *  The number can be customised using experiment.run.prefix, which should be set to a power of 10.\n
     *  A single run can be specified by setting experiment.run.number - if this coincides with an existing run/directory\n
     *  then the files there may be overwritten (unless explicit output file names are changed).
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied*/
    void setOutputFilePaths(parameterSettings& parameters){
        //naming convention for output files
        _filePrefix=   parameters.get("experiment.output.directory")+"/"+parameters.get("experiment.name");
        if (!std::filesystem::exists(_filePrefix))std::filesystem::create_directories(_filePrefix);
        std::string runNumber= parameters.get("experiment.run.number");
        std::string m00="/run_";
        if (runNumber!="-1"){
            m00=m00+runNumber;
        }else{
            //auto-increment run number if run.number is not set
            int i=0;
            //allow for userdefined number of total runs - expected to be a power of ten
            std::string zeros=parameters("experiment.run.prefix");
            //add just the zero part (strip off expected leading 1)
            m00="/run_"+zeros.substr(1);
            // Find a new directory name - then next in numerical order
            while(std::filesystem::exists(_filePrefix+m00)){    
                i++;
                std::stringstream ss;
                int zeroIndex=stoi(zeros)/10;while(zeroIndex>i && zeroIndex>1){ss<<"0";zeroIndex=zeroIndex/10;}
                ss<<i;
                runNumber=ss.str();
                m00="/run_"+runNumber;
            }
        }
        if (!std::filesystem::exists(_filePrefix+m00))std::filesystem::create_directories(_filePrefix+m00);
        parameters.setParameter("experiment.run.number",runNumber);
        _filePrefix= _filePrefix+m00+"/";
        _filePostfix="";
        std::cout<<"Outputfiles will be named "<<_filePrefix<<"<Data Name>"<<_filePostfix<<".<filenameExtension>"<<std::endl;
        parameters.saveParameters(_filePrefix);
    }
    //------------------------------------------------------------------------
    /** @brief Set up the agents and places, and allocate agents to homes, workplaces, vehicles. \n
     *  @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied
     *  @details The relative structure of the places, size homes and workplaces and the number and size of transport vehicles, together with the schedule, \n
     *  will jointly determine how effective the disease is a spreading, given the contamination rate and recovery timescale \n
     *  This simple intializer puts three agents in each home, 10 agents in each workplace and 30 in each bus - so agents will mix in workplaces, home and buses in slightly different patterns.
     */
    void init(parameterSettings& parameters,std::string domain){
        timeStep::reportDate();
        modelFactory& F=modelFactorySelector::select(parameters("model.type"));
        //create the distribution of agents, places and transport
        F.createAgents(parameters,agents,places,domain);
        //set off the disease! - some number of agents (default 1) is infected at the start.
        //shuffle things so agents are allocated at random
        random_shuffle(agents.begin(),agents.end());
        long num=std::min((long)parameters.get<long>("disease.simplistic.initialNumberInfected"),(long)agents.size());
        for (long i=0;i<num;i++)agents[i]->becomeInfected();
        
        std::cout<<"Day number 0=mon: "<<timeStep::findWeekDay(2022,2,23)<<std::endl;
        exit (0);
    }
    //------------------------------------------------------------------------
    /** @brief Finish off model including any final output etc. \n
     *  @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied
     *  @details Right now this is here just to output final values at the end of the last step.
     */
    void end(parameterSettings& parameters){
       long infected=0,recovered=0,dead=0;
        //accumulate totals - at the start of the step - so the step 0 is initial data
       #pragma omp parallel for reduction(+:infected,recovered,dead)
        for (long i=0;i<agents.size();i++){
            if (agents[i]->alive()){
                if (agents[i]->diseased())infected++;
                if (agents[i]->recovered())recovered++;
            }else{
                dead++;
            }
        }
        //output a summary .csv file
        int stepNumber=parameters.get<int>("run.nSteps");
        output<<stepNumber<<","<<stepNumber*timeStep::hoursPerTimeStep()<<","<<agents.size()-infected-recovered-dead<<","<<infected<<","<<recovered<<","<<dead<<std::endl;
        
    }
    //------------------------------------------------------------------------
    /** @brief Advance the model time step \n
    *   @details split up the timestep into update of places, contamination of places by agents, infection and progress of disease and finally update of agent locations \n
        These loops are separated so they can be individually timed and so that they can in principle be individually parallelised with openMP \n
        Also to avoid any systematic biases, agents need to all finish their contamination step before any can get infected. 
        @param stepNumber The timestep number passed in from the model class
        @param parameters A \b reference to a class that holds all the possible parameter settings for the model.\n Using a reference ensures the values don't need to be copied*/
    void step(int stepNumber, parameterSettings& parameters){

#ifdef COUPLER
        //If using the MUI coupler, exchange data. agents may leave to become travellers, and travellers may return
        coupler->exchange(stepNumber,agents,travellers,leavers);
#endif
        //count tests whether anything needs to be exchanged with the coupler *from* this domain - still need to run coupler to check for arrivals
        leavers=false;

        //Note where travellers are referred to, these include ONLY agents that have travelled to here from another MUI domain
        
        //set some timers so loop relative times can be compared - note disease loop tends to get slower as more agents get infected.

        auto start=timeReporter::getTime();
        auto end=start;


        //timereporters are used to check how long parts of the model take to run...at least for the first step
        if (stepNumber==0)start=timeReporter::getTime();
        //counts the totals
        long infected=0,recovered=0,dead=0;
        //accumulate totals - at the start of the step - so the step 0 is initial data
        //NB in very large runs (100s of millions of agents) this becomes very inefficient - so use a reduction
        #pragma omp parallel for reduction(+:infected,recovered,dead) 
        for (long i=0;i<agents.size();i++){
            if (agents[i]->active()){
                if (agents[i]->alive()){
                    if (agents[i]->diseased())infected++;
                    if (agents[i]->recovered())recovered++;
                }else{
                    dead++;
                }
            }
        }
        //travellers have come here from a remote MPI domain
        #pragma omp parallel for reduction(+:infected,recovered,dead) 
        for (long i=0;i<travellers.size();i++){
            if (travellers[i]->active()){
                if (travellers[i]->alive()){
                    if (travellers[i]->diseased())infected++;
                    if (travellers[i]->recovered())recovered++;
                }else{
                    dead++;
                }
            }
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time on accumulating disease totals: ",start,end);
            start=end;
        }
        //output a summary .csv file
        output<<stepNumber<<","<<stepNumber*timeStep::hoursPerTimeStep()<<","<<agents.size()-infected-recovered-dead<<","<<infected<<","<<recovered<<","<<dead<<std::endl;
        //update the places - changes contamination level
        //note the pragma statement here allows openmp to parallelise this loop over several threads 
        #pragma omp parallel for
        for (long i=0;i<places.size();i++){
            places[i]->update();
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Time updating places: ",start,end);
            start=end;
        }
        //do disease - synchronous update (i.e. all agents contaminate before getting infected) so that no agent gets to infect ahead of others.
        //alternatively could be randomized...depends on the idea of how a location works...places could be sub-divided to mimic spatial extent for example.
        #pragma omp parallel for
        for (long i=0;i<agents.size();i++){
            if (agents[i]->active())agents[i]->cough();
        }
        #pragma omp parallel for
        for (long i=0;i<travellers.size();i++){
            if (travellers[i]->active())travellers[i]->cough();
        }
        if(stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time coughing: ",start,end);
            start=end;
        }
        //the disease progresses
        //This is faster here using an RNG separate for each thread
        #pragma omp parallel for
        for (long i=0;i<agents.size();i++){
            if (agents[i]->active())agents[i]->process_disease(randoms[omp_get_thread_num()]);
            //agents[i]->process_disease(randoms[0]);
        }
        #pragma omp parallel for
        for (long i=0;i<travellers.size();i++){
            if (travellers[i]->active())travellers[i]->process_disease(randoms[omp_get_thread_num()]);
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time being diseased: ",start,end);
            start=end;
        }
        //move around, do other things in a location
        // if either agents or travellers indicate they want to leave the domain at the start of the next step, set leavers flag.
        #pragma omp parallel for 
        for (long i=0;i<agents.size();i++){
            if (agents[i]->active()){
                agents[i]->update();
                if (agents[i]->leaver()) leavers=true;;
            }
        }
        #pragma omp parallel for 
        for (long i=0;i<travellers.size();i++){
            if (travellers[i]->active()){
                travellers[i]->update();
                if (travellers[i]->leaver()) leavers=true;
            }
        }
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time updating agents: ",start,end);
            start=end;
        }
        //show the step number every 10 steps
        if (stepNumber==0){
            end=timeReporter::getTime();
            timeReporter::showInterval("Run time on file I/O: ",start,end);
        }
      
        //show places - just for testing really so commented out at present
        for (long i=0;i<places.size();i++){
            //places[i]->show();
        }
        //The timestep class needs to know the current time step so that this can be used in thing like calculating the day of the week
        timeStep::update();
    }
    /** @brief report current number of agents in the model - includes both active and inactive */
    unsigned long numberOfAgents(){
        return agents.size();
    }
    /** @brief report current number of places in the model*/
    unsigned long numberOfPlaces(){
        return places.size();
    }
    /** @brief report current number of infections*/
    unsigned long numberDiseased(){
        unsigned long n=0;
        for (auto a:agents)    if (a->active() && a->diseased()) n++;
        for (auto a:travellers)if (a->active() && a->diseased()) n++;
        return n;
    }
    
};
#endif // MODEL_H_INCLUDED
