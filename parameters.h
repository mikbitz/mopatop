#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED
/* A program to model agents moving between places
 *    Copyright (C) 2021  Mike Bithell
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 * @file parameters.h 
 * @brief File containing the definition of the parameterSettings class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/

#include<fstream>
#include<algorithm>
#include<cctype>
#include<assert.h>
#include<map>
#include<iostream>
#include<chrono>
/**
 * @brief ParameterSettings is a class designed to hold all the parameters for the model
 * @details Parameter defaults are set here, but the may be overridden by reading from the parameterfile .\n
 * All parameters are strings (with the idea that arbitrary strings can be read from a parameter file), so need to be converted at point of use - \n
 * however, returning different data types depending on parameter name is a little tricky...this is done by having an overloaded \ref get method\n
 * the correct get method can then be called depending on the the parameter type required (e.g. int, float).\n
 * The possible types are kept by the typeID enum - for each parameter, the type can then be stored in \ref _parameterType - these are \n
 * then checked by the get method to make sure that there is no unexpected attempt to convert the type incorrectly.
 */
class parameterSettings{
    /** @brief a list of names that can be used in the parameterType map 
     *  @details to avoid c++ reserved words just the first letter is kept - e.g. i==int, d==double, f==float - see the \ref get methods */
    enum typeID{s,f,d,u,l,i,b};
    /** @brief a map from parameter names to parameter values, all currently strings */
    std::map<std::string,std::string>_parameters;
    /** @brief stores the expected type of the relevant parameter */
    std::map<std::string,typeID>_parameterType;
    /** @brief name of the input parameter file used */
    std::string _parameterFileName;
    
    //------------------------------------------------------------------------
    /** @brief function to check whether a requiested parameter name exists.\n
     *        If not exit the program.        */
    bool is_valid(std::string s){
        auto it = _parameters.find(s);
        //check that the parameter exist
        if(it == _parameters.end()){
            std::cout<<"Invalid parameter: "<<s<<std::endl ;
            exit(1);
        }
        return true;
    }
public:
    //------------------------------------------------------------------------
    /** @brief the constructor - set up defaults. A filename is \b required when the parameterSettings object is created
     *        @details Currently you could set up several objects like this, with different input files, so take care that the filename is correct! \n
     *        Note that only parameters whose names are defined in \ref setDefaults can be imported from the parameter file - invalid names will cause the code to exit\n
     *        Parameter types have to also be correctly set up in the \ref setDefaults method*/
    parameterSettings(){
        setDefaults();
    }
    //------------------------------------------------------------------------
    /** @brief Read in any values from the parameter file
     *    @param inputFileName A string giving the path to the input file. The code will fail if the file does not exist (but it could be empty) 
     *    @details The parameter file is a set of lines with name:value pairs on each line, using "=" to separate the two.\n
     *    lines beginning with # are ignored and can be used for comments. Any line with no "=" will also be ignored \n
     *       needs to be called from \ref main to ensure settings are captured */
    void readParameters(std::string inputFileName){
        _parameterFileName=inputFileName;
        std::cout<<"Expecting to find model parameters in file: "<<inputFileName<<std::endl;
        std::fstream infile;
        infile.open(inputFileName,std::ios::in);
        assert(!infile.fail());
        std::string label,value,line;
        while (!infile.eof()){
            std::getline( infile, line );
            
            //set # as comment character - strip all white space from the test string, so that lines can start with spaces followed by #
            std::string test=line;
            test.erase(std::remove_if(test.begin(), test.end(), ::isspace), test.end());
            //need to check end of file again as the above getline may have actually gone past it, if previous read got to the file end
            if (!infile.eof() && test[ 0 ] != '#'){
                
                //separator between parameter name and value is =
                auto pos = line.find('=');
                
                //ignore if no "=" is found - npos is the end of the string
                if (pos!=std::string::npos){
                    //get string of length pos from position zero - so "=" will not be included in the parameter name
                    label=line.substr(0,pos);
                    //get from place after "=" to the end of the line
                    value=line.substr(pos+1);
                    //set the parameter if it exists
                    if (is_valid(label)){
                        _parameters[label]=value;
                    }
                }
                
            }
        }
        printParameters();
    }
    //------------------------------------------------------------------------
    /** @brief sets the default names, values and types of the model parameters
     @details It is assumed that all parameters are initially specified as strings (to help with reading from a file)\n
     results will be cast to the specified paramter type in the appropraite get function. At the moment the code will\n
     halt if either the initial type of the parameter is not a string, or if the type requested in the get function\n
     cannot be converted to the requested type.*/
    void setDefaults(){
        std::cout<<"Setting default parameters..."<<std::endl;
        //total time steps to run for
        _parameters["model.name"]="Mopatop";_parameterType["model.name"]=s;
        //total time steps to run for
        _parameters["run.nSteps"]="1";_parameterType["run.nSteps"]=i;
        //number of agents to create
        _parameters["run.nAgents"]="600";_parameterType["run.nAgents"]=l;
        //number of OMP threads to use. increase the number here if using openmp to parallelise any loops.
        //Note number of threads needs to be <= to number of cores/threads supported on the local machine
        _parameters["run.nThreads"]="1";_parameterType["run.nThreads"]=i;
        //random seed
        _parameters["run.randomSeed"]="0";_parameterType["run.randomSeed"]=i;
        //the units for the timestep - valid are years,months,days,hours,minutes or seconds
        _parameters["timeStep.units"]="hours";_parameterType["timeStep.units"]=s;
        //the actual time duration of each step in the above units
        _parameters["timeStep.dt"]="1";_parameterType["timeStep.dt"]=d;
        //path to the output file
        _parameters["outputFile"]="diseaseSummary";_parameterType["outputFile"]=s;
        //path to location of output files
        _parameters["experiment.output.directory"]="./output";_parameterType["experiment.output.directory"]=s;
        //a name for all runs in this experiment
        _parameters["experiment.name"]="default";_parameterType["experiment.name"]=s;
        //the number of the current run
        _parameters["experiment.run.number"]="-1";_parameterType["experiment.run.number"]=i;
        //brief indication of what this experiment is about
        _parameters["experiment.description"]="The default parameter set was used";_parameterType["experiment.description"]=s;
        //String to use when numbering runs - this default value allows for max. 10,000 runs numbered 0000 to 9999
        _parameters["experiment.run.prefix"]="10000";_parameterType["experiment.run.prefix"]=i;
        //The model version should be set by the main program, once the parameter defaults have been set
        _parameters["model.version"]="Unknown";_parameterType["model.version"]=s;
        //Number of times the run will be repeated with the same parameter set but different random seeds
        _parameters["run.nRepeats"]="1";_parameterType["run.nRepeats"]=i;
        //Number of times the run will be repeated with the same parameter set but different random seeds
        _parameters["run.randomIncrement"]="1";_parameterType["run.randomIncrement"]=i;
        //settings for the simplest possible disease parameterisation
        _parameters["disease.simplistic.recoveryRate"]="0.0007";_parameterType["disease.simplistic.recoveryRate"]=d;
        _parameters["disease.simplistic.deathRate"]="0.0007";_parameterType["disease.simplistic.deathRate"]=d;
        _parameters["disease.simplistic.infectionShedLoad"]="0.001";_parameterType["disease.simplistic.infectionShedLoad"]=d;
        _parameters["disease.simplistic.initialNumberInfected"]="1";_parameterType["disease.simplistic.initialNumberInfected"]=i;
        //decrement rate for contamination in all places
        _parameters["places.disease.simplistic.fractionalDecrement"]="1";_parameterType["places.disease.simplistic.fractionalDecrement"]=d;
        //if set this flag will cause contamination to be reset to zero every timestep
        _parameters["places.cleanContamination"]="false";_parameterType["places.cleanContamination"]=b;
        //if set this flag will cause contamination to be reset to zero every timestep
        _parameters["schedule.type"]="mobile";_parameterType["schedule.type"]=s;
        //if set this flag will cause contamination to be reset to zero every timestep
        _parameters["model.type"]="simpleMobile";_parameterType["model.type"]=s;
    }
    //------------------------------------------------------------------------
    /** @brief reset the value of an existing parameter
     *  @param name A string giving the name of the parameter
     *  @param value A string containing the new value - NB if this is to be converted later e.g. to int or float, make sure the string has a valid value!
     *        @details for example to change the random seed when a new model is created \n
     *        \code
     *        parameters.setParameter("randomSeed","10");
     *        \endcode
     *        Note that there is currently no type checking! The will lead to a fail later if the wrong \ref get method is called */
    void setParameter(std::string name,std::string value){
        std::cout<<"Setting parameter: "<<name<<" to: "<<value<<std::endl;
        is_valid(name);
        _parameters[name]=value;
    }
    //--------------------------------------------------------------------------------------------
    /** @brief Print the parameters read from the parameter file to stdout - trying to look reasonably lined up */
    void printParameters(){
        std::cout<< "Input Parameter File: "<<_parameterFileName      <<std::endl;
        std::cout<< "----------------------------- "            <<std::endl;
        std::cout<< "Successfully found parameters "            <<std::endl;
        for (auto& [label,value]:_parameters){
            //for some reason label does not come out as a std::string.
            std::string name=label;
            while(name.length()<50)name+=" ";
            std::cout<<name<<"  "<<value<<std::endl;
        }
        std::cout<< "----------------------------- "            <<std::endl;
    }
    //--------------------------------------------------------------------------------------------
    /** @brief Save the parameters read from the parameter file to a file called "RunParameters" - trying to look reasonably lined up
        @param path The current experiment and run path - call this from \ref model::setOutputFilePaths once the full path is known
        @details saving out the full parameter list automatically with each run helps to identify what went into a given set of output files\n
        and hopefully allows for results to be reproduced if necessary */
    void saveParameters(std::string path){
        std::fstream f;
        f.open(path+"RunParameters",std::ios::out);
        std::cout<< "Saving Parameters To: "<<path+  "RunParameters"    <<std::endl;
        std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        f<<"Run started at: "<<ctime(&t)<<std::endl;
        f<< "----------------------------- "            <<std::endl;
        f<< "Input Parameter File: "<<_parameterFileName     <<std::endl;
        f<< "----------------------------- "            <<std::endl;
        for (auto& [label,value]:_parameters){
            std::string name=label;
            while(name.length()<50)name+=" ";
            f<<name<<"  "<<value<<std::endl;
        }
        f<< "----------------------------- "            <<std::endl;
    }
    //------------------------------------------------------------------------
    /** @brief allow parameters to be returned using an instance of class parameters using a string
     *  @param s the name of the parameter requested.
     *   Example:-
     * \code
     * parameterSettings p;
     * std::string filename=p("outputFile");
     * \endcode
     * function fails if the requested parameter not been defined - the program halts\n
     * This version returns only the string variant of the parmeter value
     */
    std::string operator ()(std::string s){
        is_valid(s);
        return _parameters[s];
    }
    //------------------------------------------------------------------------
    /** @brief allow parameters to be returned with a given type conversion, in this case double\n
     *  @param s the name of the parameter requested.
     *  @details Since the parameters are stored as strings, but may represent other types, the get function allows\n
     *  them to be converted safely, with different function versions for each possble datatype.
     *  The enable_if statment allows the compiler to workout that this version should be used if the\n
     *  datatype, T, is set to double.
     *  Example:-
     * \code
     * parameterSettings p;
     * std::string rate=p.get<double>("rate");
     * \endcode
     * function fails if the requested parameter not been defined or the tyep is incorrect - the program halts\n
     */
    template <typename T>
    typename std::enable_if<std::is_same<T,double>::value, T>::type
    get(std::string s){
        //check the parameter name exists
        is_valid(s);
        //make sure the type is as expected
        assert(_parameterType[s]==d);
        return stod(_parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for floats\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, float>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(_parameterType[s]==f);
        return stof(_parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for int\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, int>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(_parameterType[s]==i);
        return stoi(_parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for long integers\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, long>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(_parameterType[s]==l || _parameterType[s]==i);
        return stol(_parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for unsigned integers\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, unsigned>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(_parameterType[s]==u);
        //seems there is no conversion just to unsigned, so use unsigned long 
        return stoul(_parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for boolean values integers\n
     *  @param s the name of the parameter requested.
     *  @details treat the conversion to bool carefully in case the user input something like "True "
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, bool>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(_parameterType[s]==b);
        std::string lval=_parameters[s];
        //make sure the string is all lower case and has no leadin gor trainling spaces
        std::for_each(lval.begin(), lval.end(), [](char & c) {c = std::tolower(c);});
        lval.erase(std::remove_if(lval.begin(), lval.end(), ::isspace), lval.end());
        if (lval=="true") return true;
        return false;
    }
    //------------------------------------------------------------------------
    /** @brief The default get just returns the string value\n
     *  @param s the name of the parameter requested.
     */
    std::string get(std::string s){
        is_valid(s);
        return _parameters[s];
    }
};
#endif // PARAMETERS_H_INCLUDED
