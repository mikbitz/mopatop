/**
 * @mainpage
 * @section intro_sec Introduction
 * This model is aimed at representing the patterns of movement and interaction of agents that represent individual people as they go about their daily activities. \n
 * The current version is intended to show how this can be done using a simple C++  program (using this language for speed of execution) \n
 * For this reason at the moment all code is in a single file. This can become unwieldy in a larger application though. \n
 * 
 * The current objective is to be able to model a simple disease, and to tie this to agent behaviour at the scale of an entire country.\n
 * Loop parallelisation with openMP is used to accelerate execution if needed.
 * @subsection Main Main ideas
 * Agents move between places according to a given fixed travel schedule. Places include transport vehicles. \n
 * In each place, agents with a disease can add contamination, which then decays exponentially over time \n
 * (or else it can be reset to zero at the start of each step - disease transmission then just depends on \n
 * number of agents in any location). The release of total contamination depends linearly on time spent in \n
 * each location. In a contaminated location, susceptible agents can pick up the infection - they then recover\n
 * with a fixed chance per timestep, and are subsequently immune.
 * @subsection Compiling Compiling the model
 * On a linux system with g++ installed just do \n
 * g++ -o agentModel -O3 -fopenmp -std=c++17 agent.cpp places.cpp main.cpp\n
 * Note the current version requires g++>= and c++17 in order for the filesystem function to work (for creating new directories etc.)\n
 * If using openmp (parallelised loops) then set the  number of threads in the parameterSettings class.\n
 * Note the number of cores to be used must be <= number supported by the local machine!
 * @subsection Run Running the model
 * At present this is a simple command-line application - just type the executable name (agentModel above) and then return.\n
 * This will use the default parameter file. To use an alternative specify the file name after the executable name:- \n
 * \code
 * agentModel parameterFileName
 * \endcode
 * The parameter file allows for a number of runs to be made from a single call to agentModel, but with different random seeds\n
 * Each run output will be stored in a separate directory. See the run section in the parameter file for how to set this up.
 * @subsection dett Detailed Description
 * For a formalised description of the model  see  @ref ODD
 * @page ODD ODD description
 * @section odd_intro Introduction
 * This page describes the model in more detail using the ODD+D formalism (Overview, Design Concepts and Details + Decisions) \n
 * as suggested in Grimm et al https://doi.org/10.1016/j.ecolmodel.2010.08.019 \n
 * and extended by M&uuml;ller et al https://doi.org/10.1016/j.envsoft.2013.06.003\n 
 * @section over Overview
 * This is an agent-based model of movement and respiratory infectious disease-spread.
 * @subsection purp Purpose
 * To model the spread of disease through a population of individual people as they move between \n
 * the places where they do their daily activites.The model is intended to scale to large numbers \n
 * of agents, at least the scale of a medium sized country, without requiring too much run-time.
 * @subsection enti Entities, State variables and Scales
 * @subsubsection sp space
 * The model currently has no notion of spatial scale, except as implicitly represented by time \n
 * spent in vehicles when travelling between places, Individual places have no internal spatial\n
 * structure - they are assumed completely mixed, so that all agents can in principle access all others.
 * @subsubsection ti time
 * There is a time step, representing a customiseable unit of real-world time (see \ref timestep.h). Every place and agent and disease \n
 * is updated synchronously at each step. This ensures that a)Updates to contamination take place regularly\n
 * even if there are no agents present b)Agents do not need to co-ordinate in terms of times spent in places\n
 * (which otherwise require o(n squared) interactions in each place), and can meet by chance within a timestep\n
 * c)outputs are easily set up at regular times. Care is required to make sure this does not lead to agents\n
 * preferentially always "acting first", so may either require the order of agents activity to be randomised, or\n
 * for particular activities to be completed for ALL agents before subsequent ones (e.g.all agents need to complete\n
 * coughing in a place before any are allowed to update their infection status)\n
 * However, agents also have an event-based schedule - so the regular timestep does not preclude agents having \n
 * heterogenous activities with respect to travel or other actions, provided that the schedule time-grain is not shorter\n
 * than the main timestep (or the schedule has its own sub-time steps for taking forward an activity).
 * @subsubsection rd random numbers
 * A single pseudo random number sequence is generated using the mersenne twister algorithm
 * @subsubsection tr benchmarking
 * A simple \ref timeReporter class is set up that can be used to report the run-time of different parts of the model\n
 * Useful for benchmarking and understanding which parts are the most comu[puationally expensive.
 * @subsubsection pa parameters
 * All of the model parameters are obtained from a parameter file \n
 * They are all collected into a single \ref parameterSettings class - this includes things like\n
 * random seed, number of agents, number of steps to run for. To include a new parameter, add it to the code\n
 * in the \ref parameterSettings::setDefaults method in \ref parameters.h. You can then configure it in the parameter file\n
 * The parameter file name is by default (!) defaultParameterFile.\n
 * Multiple repeat runs with different random seeds but all other parameters the same can be specified from the parameter file.\n
 * Experiments can be set up in the parameter file so that output from each run goes automatically into a separate directory, \n
 * with the paramters used stored along with the output \n
 * See the documentation in defaultParameterFile for details.
 * @subsubsection pl places
 * Places are at present simple containers that can take any number of agents. Agents keep a flag pointing to their\n
 * current place, so that they can add contamination, or examine the contamination level inorder to become infected.\n
 * Places are also containers for the contamination, and evolve its level over time - current;y is just decreases exponentially.\n
 * In a more complex model places could have substructure (e.g. a canteen, multiple buildings in a hospital) and a \n
 * topology (which building is next to which other, are there floors with differnt access) and geometry (how far away are \n
 * other buildings).\n
 * Note that as a result of the travel Schedule there is an implicit topology of connections between places, and an associated\n
 * implict induced social network, beyond that which would exist purely in households\n
 * At present there are three types of place - home, work and vehicle - others could be added (e.g. shops, hospitals).
 * @subsubsection ag agents
 * Agents are individual people with just a few attributes - their current place, whether they are infected or immune, \n
 * a travel schedule, and a count-down to the next travel-schedule event. They also have a list of known places \n
 * (home, work,vehicle) which can differ for each agent - in practice some of them will share a home, a workplace, or a \n
 * travel vehicle (e.g. a bus), and this allows for the spread of the disease.
 * @subsubsection di disease
 * A single simple disease allows agents interrogate it and discover whether they to recover at a fixed average\n
 * rate, to be infected if there is contamination and to shed infection into a place, again at a rate per unit time.\n 
 * @subsubsection ts travelSchedule
 * Each agent has their own copy of a travel schedule - at present, though, this can only be switched between\n
 * two types - one where the agents moves between a fixed set of places, and one where they remain permanently at home\n
 * The latter allows e.g. testing against a fully mixed disease model, if home is the same place for all agents.\n
 * The schedule has an ordered list of place types representing destination for travel, and a time to be spent\n
 * in each place of a given type. Agents use the place type to select their own particular place to travel to \n
 * when the scheduled time in their current place has expired.
 * @subsubsection moo model
 * Various different model types can be selected with different behaviours. These tend to have different configurations\n
 * of places, travel and home, and different schedules for moveing between these. For example, there is a model\n
 * that intialises all agents to exist in a single place, and with a schedule that leads them all to remain there for\n
 * the whole model execution. Switching between models uses the Factory pattern to allow different kinds of models to be chosen.
 * @subsection proc Process Overview and Scheduling
 * @subsection up update places
 * At the start of the timestep all places update their contamination level
 * @subsection cu cough
 * Once places are updated, all agents with disease shed contamination into their current place.
 * @subsection pds progress disease
 * After the contamination step completes, agents get the chance to recover, or to be infected if susceptible.
 * @subsection ua update agents
 * Other activities can now take place (placeholders functions exist for actions at home, at work or in transit)
 * @section desi Design concepts
 * @subsection basi Basic Principles
 * Infectious diseases are spread by individuals shedding infectious material into an environment.\n
 * In the case of respiratory diseases this is by coughing or sneezing, which distributes infectious \n
 * droplets into the space they currently occupy. Over time, if they stay in the same place, and it \n
 * is not well ventilated, then the infectiousness of the place may increase (especially if, for example, \n
 * air-conditioning keeps particles suspended). However, such particles will gradually settle out onto \n
 * surfaces, and then lose their infecitousness over time.Other susceptible individuals moving into the \n
 * contaminated space may become infected, possibly some time after the original infectious agent has left.\n
 * Once individuals are infected, they too can spread the disease, but they have a constant rate per unit\n
 * time possibility of recovering.Recovered individuals acquire some level of immunity to re-infection, that\n
 * eventually may cause the disease to disappear. \n
 * People travel between different locations during their daily activity, and these places may have \n
 * different occupancy levels, depending on the type of activity they represent.These places are in \n
 * many cases largely separated from other locations in terms of the possibility of spreading disease,\n
 * so act to an extent as closed environments in which disease can spread.Such places include collectively\n
 * shared transport, such as buses trains and cars (but not, for example, bicycles, except maybe tandems).\n
 * The spatial relationship of these locations only matters to the extent that this determines how long \n
 * people spend in transport vehicles, and what potential there is for moving between locations(e.g. \n 
 * whether there are nearby restaurants, coffee shops etc.)
 * @subsection theo Theoretical and Empirical Background
 * The model is related to the classical SIR(D) compartmental disease models, but varies from these in that the \n
 * population is not assumed to be completely mixed (although one can emulate this by having a single place for\n
 * agents to occupy, and thereby check this model against a SIR model, since places in this model are currently\n
 * assumed completely mixed internally). Respiratory infectious diseases such as COVID-19 seem to depend not just\n
 * on the characteristics of individual, but what kind of space is occupied, how long people spend jointly in the\n
 * space, and how confined it is. 
 * @subsection indiv Individual Decision Making
 * Agents decide to move to a new place when the time in the current place expires (as set by the travel schedule).
 * @subsection lear Learning
 * None.
 * @subsection indis Individual sensing
 * Agents detect the level of contamination at the current location.
 * @subsection indip Individual prediction
 * None.
 * @subsection objec Objectives
 * Agents just follow a fixed travel schedule.
 * @subsection inte Interaction
 * Interaction is indirect through contamination levels in the various places
 * @subsection colle Collectives
 * Agents collectively occupy places such as work, home and transport, at different times.
 * @subsection emer Emergence
 * The progress of the disease emerges as a result of contamination of places and subsequent catching of infection by agents
 * @subsection adap Adaptation
 * None.
 * @subsection hete Heterogeneity
 * Agents differ by their home, work and transport places, and whether they currently have the disease.
 * Places differ by contamination level.
 * @subsection stoc Stochasticity
 * The initial work places for agents are assigned at random irrespective of home address.\n
 * Disease infection and recovery are controlled by pseudo-random numbers.
 * @subsection obse Observation
 * Total numbers susceptible, infected and recovered are output to a csv file, along with the timestep
 * @section imple Implementation Details
 * @subsection init Initialisation
 * This is crucial to the model behaviour, as it determines the connections between places and\n
 * defines where agents regard their known places to be. In the current version, with all agents \n
 * stay in the same place, and have a single home location, or else:-\n
 * - First homes are created - enough for 3 agents per home.\n
 * - Agents are then created and allocated 3 to a home untilt the list of agents is exhausted.\n
 * - Work places are then created, enough for 10 agents per workplace.\n
 * - The agents list is shuffled randomly, then agents are allocated to work places 10 at a time in order.\n
 * - Vehicles are created representing buses, with a capacity of 30 agents. Agents are allocated to\n 
 * buses in the same order as to work places\n
 * - Travel schedules are initialised with Agents on the bus heading home - every agents has the same\n
 * schedule with the exact same time spent in each place.\n
 * - A (customiseable) number of agents are infected at random with the disease\n
 * @subsection inpu Input Data
 * None.
 * @subsection subm Submodels
 * @subsubsection dise Disease
 * - Agents are infected if contamination > a uniform random number between 0 and 1, and not immune\n
 * - Agents recover if infected and recovery rate > a uniform random number between 0 and 1 \n
 * - Agents die if infected and death rate > a uniform random number between 0 and 1 \n
 * - Agents add a fixed value \ref disease::infectionShedLoad to a place, if infected.
 * @subsubsection cont Contamination
 * - Places have a contamination level which is added to when agents local to the place cough.\n
 * - The level then decreases at a fixed rate over time.\n
 * \f$\frac {dc}{dt} = \sum_{agents}infectionShedLoad-fractionalDecrement * c\f$ \n
 * - Places can have contamination reset to zero - if this is done every timestep, but before\n
 * agents cough, then the level just becomes proportional to the number of agents in a place.
 * @subsubsection trvl travel
 * - Agents set a counter whenever they move, which is the time to be spent in the place the move to\n
 * - Every time step, the counter is decremented by the time step size. Once the counter reaches zero\n
 * agents query the travel schedule for the next place type to move to and then go there. It is assumed that\n
 * the time set in the schedule is an exact multiple of the timestep
 * - Structure is included to allow agents to add and remove themsleves from a list of agents in each place\n
 * but at present this is not needed (and the add and remove process is computationally quite expensive).
 */
