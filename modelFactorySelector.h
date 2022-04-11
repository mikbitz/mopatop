#ifndef MODELFACTORYSELECTOR_H
#define MODELFACTORYSELECTOR_H
class modelFactory;
#include <string>
/** @brief A class to pick one of a number of possible agent factories 
    @details This is a static class used to define a pointer to a \ref modelFactory \n
    Each model factory can be selected using a name passed into the \ref select method using\n
    a string. This is used in the model class along with the \ref parameterSettings to choose\n
    what the model initialization of places and agents will look like.
   */
class modelFactorySelector{
public:
    /** @brief choose the model factory
     * @param name A string that names one  \ref modelFactory
     * @return A reference to the requested  \ref modelFactory
     * Example use:-
     \code
        modelFactory& F=modelFactorySelector::select(parameters("model.type"));
        //create the distribution of agents, places and transport
        F.createAgents(parameters,agents,places);
     \endcode
     */
    static modelFactory&  select(std::string name);
};
#endif
