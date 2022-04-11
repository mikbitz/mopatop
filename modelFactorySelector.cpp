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
 * @file modelFactorySelector.cpp 
 * @brief File containing list of model factories that can be selected
 * 
 * @author Mike Bithell
 * @date 11/04/2022
 **/
#include "modelFactory.h"
#include "modelFactorySelector.h"
modelFactory&  modelFactorySelector::select(std::string name){
    modelFactory* F=nullptr;
    if (name=="simpleOnePlace")F=new simpleOnePlaceFactory();
    if (name=="simpleMobile")  F=new simpleMobileFactory();
    if (name=="flying")  F=new flyingFactory();
    if (F==nullptr)std::cout<<"Name "<<name<<" not recognised in modelFactorySelector"<<std::endl;
    assert(F!=nullptr);
    return *F;
}
