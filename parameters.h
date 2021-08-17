/**
 * @brief ParameterSettings is a class designed to hold all the parameters for the model
 * @details At present these are hard coded here, but more usefully these could be delegated to an input file.
 * At the moment all parameters are strings (with the idea that arbitrary strings can be read from a parameter file), so need to be converted at point of use - \n
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
    /** @brief function to check whether a requiested parameter name exists.\n
        If not exit the program.
        */
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
    parameterSettings(){

    //total time steps to run for
    parameters["nSteps"]="4800";parameterType["nSteps"]=i;
    //number of agents to create
    parameters["nAgents"]="600";parameterType["nAgents"]=i;
    //number of OMP threads to use increase the number here if using openmp to parallelise any loops.
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
 *  them to be converted safely, with different function versions for each possble datatype
 *   Example:-
 * \code
 * parameterSettings p;
 * std::string rate=p.get<double>("rate");
 * \endcode
 * function fails if the requested parameter not been defined - the program halts\n
 */
    template <typename T>
    typename std::enable_if<std::is_same<T,double>::value, T>::type
    get(std::string s){
        is_valid(s);
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
