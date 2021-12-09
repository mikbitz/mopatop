#ifndef REMOTE_TRAVEL_H_INCLUDED
#define REMOTE_TRAVEL_H_INCLUDED
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
 * @file remoteTravel.h 
 * @brief File containing the definition of the remoteTravel class
 * 
 * @author Mike Bithell
 * @date 10/12/2021
 **/
//------------------------------------------------------------------------
/**
 * @brief The class defines a remote place to stay, such as a hotel, and a vehicle in which to travel there (e.g. a plane)
 * @details At the moment the assumption is that this is effectively a holiday, where agents will stay at a single hotel\n
 *          with a single way to get there - this means that at present one can't travel to a hotel other than by a single fixed travel vehicle\n
 *          however, agents could accumulate at a single hotel by travelling there at different times. \n
 *          The hotel currently has infinte capacity. Hilbert would be pleased. A capacity test could be included by adding a counter in the visit method\n
 *          

*/
class remoteTravel {
        /** @brief vehicle for travelling to a remote location */
        place* plane;
        /** @brief place to stay while away */
        place* hotel;
        /** @brief flag to say if this place is on a remote MPI domain
            @details All places are defined on every domain, so that travellers can find out about them, but also so that they can be used for return trips from the remote domain*/
        bool _remoteDomain;
public:
        /** @brief default constructor is an empty place - agents should not be allowed to travel here! */
        remoteTravel(){_remoteDomain=false;plane=nullptr;hotel=nullptr;}
        /** @brief Sets up a travel location and transport, and adds its places to the place list (so that the places update function has effect)  
            @param parameters the model parameters, used in setting up a place
            @param places the vector of all places
            @param remote set the flag for a remote MPI domain - defaults to false if not set by the caller*/
        remoteTravel(parameterSettings& parameters,std::vector<place*>& places,bool remote=false):_remoteDomain(remote){
            plane=new place(parameters);plane->setID(places.size());places.push_back(plane);
            hotel=new place(parameters);hotel->setID(places.size());places.push_back(hotel);
        }
        /** @brief agents that visit here need to know about the transport and location 
            @details Agents call this to set up their visit. Their travel schedule is expected to be set consistently. \n
            It is expected that agents will take care of remebering their home location and travel mode, so that these can be \n
                     reset once a visit is complete
            @param a a pointer to an agent */
        void visit(agent* a){
            a->setTransport(plane);
            a->setHome(hotel);
        }
        /** @brief Note whether the location is on a remote domain
         *  @details all travel locations should exist on all domains, so that agents can set up a visit, and \n
         *           so that they can manage a return trip on a local plane. Named locations therefore need to be setup on each\n
                     domain consistently (e.g. so that "London" is remote on one domain, but local on another)*/ 
        bool isOnRemoteDomain(){
            return _remoteDomain;
        }
};
#endif
