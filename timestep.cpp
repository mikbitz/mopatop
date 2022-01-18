//------------------------------------------------------------------------
//------------------------------------------------------------------------
/**
 * @file timestep.cpp 
 * @brief File to define static variables for timestep
 * 
 * @author Mike Bithell
 * @date 17/08/2021
 **/
#include "timestep.h"
//setup dt to be 1 hour if nothing else is specified
double timeStep::years=24*3600*365;
double timeStep::months=24*3600*30;
double timeStep::days=24*3600;
double timeStep::hours=3600;
double timeStep::minutes=60;
double timeStep::seconds=1;
double timeStep::dt=3600;
std::string timeStep::units="hours";
int timeStep::stepNumber=0;
