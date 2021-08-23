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
#ifndef PARAMETERS_H_INCLUDED
#define PARAMETERS_H_INCLUDED
#include<fstream>
#include<algorithm>
#include<cctype>
/**
 * @brief ParameterSettings is a class designed to hold all the parameters for the model
 * @details At present these are hard coded here, but more usefully these could be delegated to an input file.\n
 * All parameters are strings (with the idea that arbitrary strings can be read from a parameter file), so need to be converted at point of use - \n
 * however, returning different data types depending on parameter name is a little tricky...this is done by having an overloaded \ref get method\n
 * the correct get method can then be called depending on the the parameter type required (e.g. int, float).\n
 * The possible types are kept by the typeID enum - for each parameter, the type can then be stored in \ref parameterType - these are \n
 * then checked by the get method to make sure that there is no unexpected attempt to convert the type incorrectly.
 */
class parameterSettings{
    /** @brief a list of names that can be used in the parameterType map 
     *  @details to avoid c++ reserved words just the first letter is kept - e.g. i==int, d==double, f==float - see the \ref get methods */
    enum typeID{s,f,d,u,l,i};
    /** @brief a map from parameter names to parameter values, all currently strings */
    std::map<std::string,std::string>parameters;
    /** @brief stores the expected type of the relevant parameter */
    std::map<std::string,typeID>parameterType;
    /** @brief name of the input parameter file used */
    std::string _parameterFileName;
    
    //------------------------------------------------------------------------
    /** @brief function to check whether a requiested parameter name exists.\n
     *        If not exit the program.        */
    bool is_valid(std::string s){
        auto it = parameters.find(s);
        //check that the parameter exist
        if(it == parameters.end()){
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
     *    @details The parameter file is a set of lines with name:value pairs on each line, using ":" to separate the two.\n
     *    lines beginning with # are ignored and can be used for comments. Any line with no ":" will also be ignored \n
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
                
                //separator between parameter name and value is :
                auto pos = line.find('=');
                
                //ignore if no "=" is found - npos is the end of the string
                if (pos!=std::string::npos){
                    //get string of length pos from position zero - so "=" will not be included in the parameter name
                    label=line.substr(0,pos);
                    //get from place after "=" to the end of the line
                    value=line.substr(pos+1);
                    //set the parameter if it exists
                    if (is_valid(label)){
                        parameters[label]=value;
                    }
                }
                
            }
        }
        printParameters();
    }
    //Notes
    // convert string to lower case (use std::toupper for upper case) using a lamba function 
    //std::for_each(s.begin(), s.end(), [](char & c) {c = std::tolower(c);});
    //------------------------------------------------------------------------
    /** @brief sets the default names, values and types of the model parameters
     @details It is assumed that all parameters are initially specified as strings (to help with reading from a file)\n
     results will be cast to the specified paramter type in the appropraite get function. At the moment the code will\n
     halt if either the initial type of the parameter is not a string, or if the type requested in the get function\n
     cannot be converted to the requested type.*/
    void setDefaults(){
        std::cout<<"Setting default parameters..."<<std::endl;
        //total time steps to run for
        parameters["run.nSteps"]="1";parameterType["run.nSteps"]=i;
        //number of agents to create
        parameters["run.nAgents"]="600";parameterType["run.nAgents"]=l;
        //number of OMP threads to use. increase the number here if using openmp to parallelise any loops.
        //Note number of threads needs to be <= to number of cores/threads supported on the local machine
        parameters["run.nThreads"]="1";parameterType["run.nThreads"]=i;
        //random seed
        parameters["run.randomSeed"]="0";parameterType["run.randomSeed"]=i;
        //path to the output file
        parameters["outputFile"]="diseaseSummary.csv";parameterType["outputFile"]=s;
        //path to location of output files
        parameters["experiment.output.directory"]="./output";parameterType["experiment.output.directory"]=s;
        //a name for all runs in this experiment
        parameters["experiment.name"]="default";parameterType["experiment.name"]=s;
        //the number of the current run
        parameters["experiment.run.number"]="";parameterType["experiment.run.number"]=i;
        //brief indication of what this experiment is about
        parameters["experiment.description"]="The default parameter set was used";parameterType["experiment.description"]=s;
        //String to use when numbering runs - this default value allows for max. 10,000 runs numbered 0000 to 9999
        parameters["experiment.run.prefix"]="10000";parameterType["experiment.run.prefix"]=i;
        //The model version should be set by the main program, once the parameter defaults have been set
        parameters["model.version"]="Unknown";parameterType["model.version"]=s;
        //Number of times the run will be repeated with the same parameter set but different random seeds
        parameters["run.nRepeats"]="1";parameterType["run.nRepeats"]=i;
        //Number of times the run will be repeated with the same parameter set but different random seeds
        parameters["run.randomIncrement"]="1";parameterType["run.randomIncrement"]=i;
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
        parameters[name]=value;
    }
    //--------------------------------------------------------------------------------------------
    /** @brief Print the parameters read from the parameter file to stdout - trying to look reasonably lined up */
    void printParameters(){
        std::cout<< "Input Parameter File: "<<_parameterFileName      <<std::endl;
        std::cout<< "----------------------------- "            <<std::endl;
        std::cout<< "Successfully found parameters "            <<std::endl;
        for (auto& [label,value]:parameters){
            std::string name=label;
            while(name.length()<30)name+=" ";
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
        std::time_t t=std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
        f<<"Run started at: "<<ctime(&t)<<std::endl;
        f<< "----------------------------- "            <<std::endl;
        f<< "Input Parameter File: "<<_parameterFileName     <<std::endl;
        f<< "----------------------------- "            <<std::endl;
        for (auto& [label,value]:parameters){
            std::string name=label;
            while(name.length()<30)name+=" ";
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
        return parameters[s];
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
        assert(parameterType[s]==d);
        return stod(parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for floats\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, float>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(parameterType[s]==f);
        return stof(parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for int\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, int>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(parameterType[s]==i);
        return stoi(parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for long integers\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, long>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(parameterType[s]==l || parameterType[s]==i);
        return stol(parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief Specialisation of get for unsigned integers\n
     *  @param s the name of the parameter requested.
     */
    template <typename T>
    typename std::enable_if<std::is_same<T, unsigned>::value, T>::type
    get(std::string s){
        is_valid(s);
        assert(parameterType[s]==u);
        //seems there is no conversion just to unsigned, so use unsigned long
        return stoul(parameters[s]);
    }
    //------------------------------------------------------------------------
    /** @brief The default get just returns the string value\n
     *  @param s the name of the parameter requested.
     */
    std::string get(std::string s){
        is_valid(s);
        return parameters[s];
    }
};
#endif // PARAMETERS_H_INCLUDED
