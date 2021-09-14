#ifndef RANDOMIZER_H_INCLUDED
#define RANDOMIZER_H_INCLUDED
/* A program to model agents moveing between places
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
 * @file randomizer.h 
 * @brief File containing a wrapper class for random numbers \ref randomizer
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/


#include <random>
//------------------------------------------------------------------------
/**
 * @brief Set up a wrapper class that will provide uniform pseudo-random numbers between 0 and 1 \n
 * @details As usual with random number generators, the sequence is actually periodic with a very long period.
 * By setting the seed one can generate the same sequence repeatedly by using the same seed.
 * The C++ random number generators are rather complicated. This class allows selection of one of the available generators \n
 * without needing to look up the detail. It generates a random double between 0 and 1 using the mersenne twister generator. \n
 * The seed defaults to 0 but can be reset with \ref setSeed - note this seems to need the twister to be completely re-created, hence the use of a pointer
 * Example:-
 * \code
 * randomizer r;
 * r.setSeed(12991);
 * double randomvalue=r.number();
 * \endcode
 * Seems to be ok on multiple threads, but takes something of a hit from being only one RNG across them - would be faster if each\n
 * thread had its own RNG but this seems tricky to get right across multiple runs,
*/
#include<iostream>
class randomizer {
public:
    /** The distribution to be generated is uniform from 0 to 1 */
    std::uniform_real_distribution<> uniform_dist;
    /**  Use mersenne twister with fixed seed as the random number engine */
    std::mt19937 twister;
public:
    /** The constructor makes the class instance - optionally provide a seed*/
    randomizer(int s=0){
        uniform_dist=std::uniform_real_distribution<> (0,1);
        std::cout<<"A randomizer was set up with seed 0" <<std::endl;
        twister.seed(s);
    }
    ~randomizer(){
      //delete twister;
    }
    /** @brief return the next pseudo-random number in the current sequence */
    double number(){
     return uniform_dist(twister);
    }
    /** Set the seed that starts off a given random sequence 
     *param s The starting integer - any value can be used that fits with the size of int*/
    void setSeed(int s){
        std::cout<<"randomizer seed set to "<<s <<std::endl;
        //delete twister;
        twister.seed(s);
    }

};
//------------------------------------------------------------------------


#endif // RANDOMIZER_H_INCLUDED
