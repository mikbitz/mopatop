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
 * @brief File containing a wrapper class for random numbers
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#ifndef RANDOMIZER_H_INCLUDED
#define RANDOMIZER_H_INCLUDED
#include <random>
//------------------------------------------------------------------------
/**
 * @brief Set up a wrapper class that will provide uniform pseudo-random numbers between 0 and 1 \n
 * @details As usual with random number generators, the sequence is actually periodic iwth a very long period.
 * By setting the seed one can generate the same sequence repeatedly by using the same seed.
 * The C++ random number generators are rather complicated. This class allows selection of one of the available generators \n
 * without needing to look up the detail. It generates a random double between 0 and 1 using the mersenne twister generator. \n
 * The seed defaults to 0 but can be reset with \ref setSeed - note this seems to need the twister to be completely re-created, hence the use of a pointer
 * Example:-
 * \code
 * randomizer r=randomizer::getInstance(0);
 * r.setSeed(12991);
 * double randomvalue=r.number();
 * \endcode
 * Uses a  map to define multiple independent generators, so there is only one random sequence across all agents for a given value of the short integer parameter i 
 * The idea is that in a multi-threaded application, each thread can have its own RNG - although note there is only one instance of twister (and hence the seed) for all.
 * Using the thread number leads to a small improvement in speed.
*/
class randomizer {
public:
    /** @brief get a reference to one of a set of random number generators.
     * If no appropriate instance yet exists, create it. \n
     * Instances are currently parameterized by omp thread number by calling omp_get_thread_num()\n
     * (which returns zero if only one thread)
     * @return A reference to the available instance
     * */
    static randomizer& getInstance(){
        short i=omp_get_thread_num();
        if (instance[i]==nullptr){
            instance[i]=new randomizer();
        }
        return *instance[i];
    }
    /** The distribution to be generated is uniform from 0 to 1 */
    std::uniform_real_distribution<> uniform_dist;
    /**  Use mersenne twister with fixed seed as the random number engine */
    std::mt19937* twister;
public:
    //~randomizer(){
    //  clean();
   // }
    /** @brief return the next pseudo-random number in the current sequence */
    double number(){
     return uniform_dist(*twister);
    }
    /** Set the seed that starts off a given random sequence 
     *param s The starting integer - any value can be used that fits with the size of int*/
    void setSeed(int s){
        std::cout<<"randomizer seed set to "<<s <<std::endl;
        delete twister;
        twister=new std::mt19937(s);
    }
private:
    /** The instance of this class. As this is a singleton (there can only ever be one of this class anywhere in the code) the actual instance is hidden from the user of the class */
    static std::map<short,randomizer*> instance;
    /** The constructor makes the class instance - again private so that access can be controlled. The class is used through the getInstance method */
    randomizer(){
        uniform_dist=std::uniform_real_distribution<> (0,1);
        std::cout<<"A randomizer was set up with seed 0" <<std::endl;
        twister=new std::mt19937(0);
    }
    /** if needed, clean up the pointer - however, given there is only one, it will just get deleted at end of program execution. */
    //void clean(){if (instance!=nullptr) {delete instance;instance=nullptr;}}
};
//------------------------------------------------------------------------
//static class members have to be initialized
std::map<short,randomizer*> randomizer::instance;
#endif // RANDOMIZER_H_INCLUDED
