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
 * @file disease.h 
 * @brief File containing the definition disease class
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#ifndef DISEASE_H_INCLUDED
#define DISEASE_H_INCLUDED
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief Simple static class to represent a very very simple disease
 * @details Use of static class allws some of the details of the disease to be abstracted here, without needed a separate disease object in every agent \n
 * The disadvantage of this is that the parameters are fixed outside teh class definition (a requiremnet of C++ for static (i.e. class-based rather than instance-based) variables) \n
 * @todo update this so that parameters can be set, e.g. from a parameter file
*/
class disease{
    /** fixed per timestep chance of recovery */
    static float recoveryRate;
    /** fixed per timespte contribution to contamination at a site */
    static float infectionShedLoad;
public:
    /** recover with a fixed chance in a given timestep - this function needs to be called every timestep by infected agents*/
    static bool recover (){
      if (recoveryRate>randomizer::getInstance().number())return true;else return false;
    }
    /** contract disease if contamination is large enough (note it could be >1) - again called very time step */
    static bool infect(float contamination){
      if (contamination >randomizer::getInstance().number()) return true; else return false;
    }
    /** contribute infection to the place if diseased - called every timestep by infected agents */
    static float shedInfection(){return infectionShedLoad;}

};
float disease::recoveryRate=0.0008;
float disease::infectionShedLoad=0.001;
#endif // DISEASE_H_INCLUDED
