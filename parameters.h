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
        @details to avoid c++ reserved words just the first letter is kept - e.g. i==int, d==double, f==float - see the \ref get methods */
    enum typeID{s,f,d,u,l,i};
    /** @brief a map from parameter names to parameter values, all currently strings */
    std::map<std::string,std::string>parameters;
    /** @brief stores the expected type of the relevant parameter */
    std::map<std::string,typeID>parameterType;
    /** @brief File handle for the input parameter file */

//------------------------------------------------------------------------
    /** @brief function to check whether a requiested parameter name exists.\n
        If not exit the program.        */
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
    /** @brief the constructor - set up defaults and then read in any other values. A filename is \b required when the parameterSettings object is created
        @param inputFileName A string giving the path to the input file. The code will fail if the file does not exist (but it could be empty) 
        @details Currently you could set up several objects like this, with different input files, so take care that the filename is correct! \n
        Note that only parameters whose names are defined in setDefaults can be imported from the parameter file - invalid names will cause the code to exit */
    parameterSettings(std::string inputFileName){
        setDefaults();
        std::cout<<"Expecting to find model parameters in file: "<<inputFileName<<std::endl;
        readParameters(inputFileName);

    }
//------------------------------------------------------------------------
/** @brief Read in any values from the parameter file
 *    @param inputFileName A string giving the path to the input file.
 *    @details The parameter file is a set of lines with name:value pairs on each line, using ":" to separate the two.\n
 *    lines beginning with # are ignored and can be used for comments. Any line with no ":" will also be ignored*/
void readParameters(std::string inputFileName){
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
                std::cout<<label<<"="<<value<<std::endl;
                //set the parameter if it exists
                if (is_valid(label)){
                    parameters[label]=value;
                }
            }
            
        }
    }
}
//Notes
//s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
// convert string to lower case -use std::toupper for upper case
//std::for_each(s.begin(), s.end(), [](char & c) {c = std::tolower(c);});
//------------------------------------------------------------------------
    /** @brief sets the default names, values and types of the model parameters */
    void setDefaults(){
        //total time steps to run for
        parameters["nSteps"]="1";parameterType["nSteps"]=i;
        //number of agents to create
        parameters["nAgents"]="600";parameterType["nAgents"]=i;
        //number of OMP threads to use. increase the number here if using openmp to parallelise any loops.
        //Note number of threads needs to be <= to number of cores/threads supported on the local machine
        parameters["nThreads"]="1";parameterType["nThreads"]=i;
        //random seed
        parameters["randomSeed"]="0";parameterType["randomSeed"]=i;
        //path to the output file
        parameters["outputFile"]="diseaseSummary.csv";parameterType["outputFile"]=s;
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
