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
#include"randomizer.h"
//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @brief Simple static class to represent a very very simple disease
 * @details Use of static class allows some of the details of the disease to be abstracted here, without needed a separate disease object in every agent \n
 * The disadvantage of this is that the parameters are fixed outside the class definition (a requirement of C++ for static (i.e. class-based rather than instance-based) variables) \n
 * but they can still be set in the constructor using a parameter file.
*/
class disease{
    /** per timestep chance of recovery */
    static float recoveryRate;
    /** per timestep contribution to contamination at a site when infected */
    static float infectionShedLoad;
    /** per timestep chance of dying */
    static float deathRate;
public:
    disease(){
        //defaults if no parameters given
        recoveryRate=0.0008;
        infectionShedLoad=0.001;
        deathRate=0.;
    }
    disease(parameterSettings p){
        recoveryRate=p.get<double>("disease.simplistic.recoveryRate");
        infectionShedLoad=p.get<double>("disease.simplistic.infectionShedLoad");
        deathRate=p.get<double>("disease.simplistic.deathRate");
    }
    /** recover with a fixed chance in a given timestep - this function needs to be called every timestep by infected agents
     @param r A random number generator created by the \ref model class*/
    static bool recover (randomizer& r){
      if (recoveryRate>r.number())return true;else return false;
    }
    /** die with a fixed chance in a given timestep - this function needs to be called every timestep by infected agents
     @param r A random number generator created by the \ref model class*/
    static bool die (randomizer& r){
      if (deathRate>r.number())return true;else return false;
    }
    /** contract disease if contamination is large enough (note it could be >1) - again called very time step
     * @param r A random number generator created by the \ref model class
     * @param contamination The disease load in the current place */
    static bool infect(float contamination,randomizer& r){
      if (contamination >r.number()) return true; else return false;
    }
    /** contribute infection to the place if diseased - called every timestep by infected agents 
     @return infectionshedLoad - the current amount of infection that an agent emits per timestep into the environment */
    static float shedInfection(){return infectionShedLoad;}

};
float disease::recoveryRate=0.0008;
float disease::deathRate=0.;
float disease::infectionShedLoad=0.001;
#endif // DISEASE_H_INCLUDED
