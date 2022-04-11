#ifndef MODELFACTORYSELECTOR_H
#define MODELFACTORYSELECTOR_H
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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @file modelFactorySelector.h 
 * @brief File containing the definition of the selector for model factories
 * 
 * @author Mike Bithell
 * @date 11/04/2022
 **/
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
