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
 * g++ -o agentModel -O3 -fopenmp -std=c++17 timestep.cpp disease.cpp agent.cpp places.cpp schedulelist.cpp main.cpp\n
 * Note that you need to have openMP available even if you only intend to run single-threaded \n
 * Better use the Makefile :-\n
 * You should first remove any pre-exiisting object or dependency files with\n
 * make clean \n
 * and then type\n
 * make\n
 * Note the current version requires g++ version>=9 and c++17 in order for the filesystem function to work (for creating new directories etc.)\n
 * @subsection Run Running the model
 * At present this is a simple command-line application - just type the executable name (agentModel above) and then return.\n
 * This will use the default parameter file. To use an alternative specify the file name after the executable name:- \n
 * \code
 * agentModel parameterFileName
 * \endcode
 * The parameter file allows for a number of runs to be made from a single call to agentModel, but with different random seeds\n
 * Each run output will be stored in a separate directory. See the run section in the parameter file for how to set this up.
 * @subsection Parallell Parallellisationalisation
 * If using openmp (parallelised loops) then set the  number of threads in the parameterSettings file.\n
 * Note the number of cores to be used must be <= number supported by the local machine!
 * The facility exists to run the model over many MPI nodes useing the MUI coupler - https://github.com/MxUI/MUI\n
 * see \ref fetchall.h \n
 * In which case you need to do:- \n
 * make WITH_MPI_COUPLER=1 \n
 * and then run the model as specified in fetchall.h. However, this is currently only likely to be necessary for very large models \n
 * (more than a few billion agents), and requires care in makign sure the model set up  works across nodes. You will also need MPI\n
 * libraries installed - current testing uses mpich 2\n
 * If using MPI you need instead something like, for testing on a single machine
 * \code
 * mpiexec -f hosts -np 1 ./agentmodel parameterFile1 domain1 : -np 1 ./agentmodel defaultparameterFile2 domain2
 * \endcode
 * where the hosts is just a single line \n 
 * \code
 * localhost\n
 * \endcode
 * This runs two copies of the model with appropriately set different parameters sets (although note you need to have matching dates and timesteps!)\n
 * Each copy runs as a multi-core openMP application, with data exchange controlled as in fetchall.h\n
 * On a multi-node HPC you will need instead:-\n
 * \code
 * mpirun -np 1 -node host1 ./agentmodel parameterFile1 domain1 : -np 1 -node host2 ./agentmodel defaultparameterFile2 domain2
 * \endcode
 * @subsection dett Detailed Description
 * For a formalised description of the model  see  @ref ODD
 * For parameter settings and builing experiments see @ref params
 * @page params Parameter Setting and Experiments
 * @section PFile Parameter File
 * The model reads in its parameters from a file specifying one parameter value per line in the form \n
 * \code
 * parameterName=value 
 * \endcode
 * where lines without and "=" or starting with a # are treated as comments. parameterNames are grouped together in terms\n
 * of common functions using a dot-separated form e.g. everything beginning "run." has to do with control of the model run\n
 * including things like run.nSteps for the number of model timesteps to be used or run.randomSeed for the initial random number seed e.g..\n
 * \code
 * run.nSteps=5000
 * \endcode
 * Where a parameter exists in the model but its value is not specified in the parameter file used, then a hard-coded default value\n
 * is used instead (see \ref parameters.h). Any parameters set in the parameter file that do not have default values, or do not exist\n
 * will cause the model to halt. The file to be used is given when running the model as the first command-line argument as \n
 * \code
 * modelexe parameterFileName \n
 * \endcode
 * Every model run writes out a file listing the parameter values used in alphabetical order to the model output directory.\n
 * In this way many experiments with different parameter settings can be run without needing to edit or re-compile the model,\n
 * and a record of the model settings is kept with the results.\n
 * 
 * @section exp Experiment Files
 * In order to systematically explore and archive  model behaviour, a facility for setting up experiments and keeping a list of them in a database\n
 * is included. The experiment database takes to form of a simple csv file listing the name of each experiment, the model version used, the experiment specification file\n
 * and a brief description - this is currently kept in a sub-directory name "experiments"\n
 * An example experiment is used to help with setup of new experiments including an example specification and base parameter file.\n
 * Two python programs, makeExperiment.py and runExperiment.py allow for new experiments to be created and run in a standardised way.\n
 * The experiment specifcation file is again a csv, this time including the experiment name, parameter file name and columns giving the\n
 * values of the parameters to be used in each model run, with the full parameter set being used for a run specified in each row of the \n
 * file. Parameters not mentioned in the specification file are assumed to take default values - any value that is varied must have a \n
 * value given for every run.\n
 * makeExperiment.py allows for a new experiment to be given a name, modelversion and description - the script then copies the example experiment\n
 * into a new directory, suitably re-named, and adds the experiment to the database. The parameter and specification files can then be edited, and the\n
 * entire run set executed using runExperiment.py (or else a full set of parameter files can be generated, in case, for exmaple, the experiment is to.\n
 * be run via a queueing system such as on HPC machines).   
 * @page ODD ODD description
 * 
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
 * However, agents also have time-based rules - so the regular timestep does not preclude agents having \n
 * heterogenous activities with respect to travel or other actions, provided that the time-grain is not shorter\n
 * than the main timestep (or the rule has its own sub-time steps for taking forward an activity).\n
 * Hours of the day and day of the week are calculated based on a model start (step 0) \n
 * @subsection dts dates
 * It is expected that a real date will be given for the model start in the form "day dd/mm/yyyy hh:mm:ss"\n
 * If no date is specified the default is Mon 01/01/1900 00:00:00 . All times are in UTC (no time zones yet)\n
 * As the model time step advances, the date also rolls forward consistently with the model time-step units \n
 * (by default one timestep corresponds to one hour). Agents can get the the current date and time in simple \n
 * 24-hour format (e.g 915, 2321), adn therefore can set actions depending on time of day, but also month/season...\n
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
 * See the documentation in defaultParameterFile for details, and \ref PFile
 * @subsubsection pl places
 * Places are at present simple containers that can take any number of agents. Agents keep a flag pointing to their\n
 * current place, so that they can add contamination, or examine the contamination level inorder to become infected.\n
 * Places are also containers for the contamination, and evolve its level over time - current;y is just decreases exponentially.\n
 * In a more complex model places could have substructure (e.g. a canteen, multiple buildings in a hospital) and a \n
 * topology (which building is next to which other, are there floors with differnt access) and geometry (how far away are \n
 * other buildings).\n
 * Note that as a result of the travel timings there is an implicit topology of connections between places, and an associated\n
 * implict induced social network, beyond that which would exist purely in households\n
 * At present there are three types of place - home, work and vehicle - others could be added (e.g. shops, hospitals).
 * @subsubsection ag agents
 * Agents are individual people with just a few attributes - their current place, whether they are infected or immune, \n
 * a set of travel rules, and switching to different activities depending on the time of day. They also have a list of known places \n
 * (home, work,vehicle) which can differ for each agent - in practice some of them will share a home, a workplace, or a \n
 * travel vehicle (e.g. a bus), and this allows for the spread of the disease.
 * @subsubsection di disease
 * A single simple disease allows agents interrogate it and discover whether they to recover at a fixed average\n
 * rate, to be infected if there is contamination and to shed infection into a place, again at a rate per unit time.\n 
 * @subsubsection ts travelSchedule
 * Each agent has their own (currently hard-coded) set of travel rules, agentsse time of day to determine the next travel event
 * In principle they could opt not to move (e.g. in lockdown) but currently they alwys do so. At present, there are only two movement\n
 * types - one where the agents moves between a fixed set of places, and one where they remain permanently at home\n
 * The latter allows e.g. testing against a fully mixed disease model, if home is the same place for all agents.\n
 * Rules implicitly define a time to be spent in each place of a given type. Agents use the place type to select \n
 * their own particular place to travel to when the scheduled time in their current place has expired.\n
 * A simple set of rules to go on holiday in the summer currently has agents fly to a hotel somewhere and stay there\n
 * for some time period.
 * @subsubsection moo model
 * Various different model types can be selected with different behaviours. These tend to have different configurations\n
 * of places, travel and home, and different patterns for moving between these. For example, there is a model\n
 * that intialises all agents to exist in a single place, and with rules that lead them all to remaining there for\n
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
 * The spatial relationship of these locations currently only matters to the extent that this determines how long \n
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
 * Agents decide to move to a new place when the time in the current place expires (as set by the appropriate rule).
 * @subsection lear Learning
 * None.
 * @subsection indis Individual sensing
 * Agents detect the level of contamination at the current location.
 * @subsection indip Individual prediction
 * None.
 * @subsection objec Objectives
 * Agents effectively just follow a fixed travel schedule.
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
 * - Agents are then created and allocated 3 to a home until the list of agents is exhausted.\n
 * - Work places are then created, enough for 10 agents per workplace.\n
 * - The agents list is shuffled randomly, then agents are allocated to work places 10 at a time in order.\n
 * - Vehicles are created representing buses, with a capacity of 30 agents. Agents are allocated to\n 
 * buses in the same order as to work places\n
 * - Travel is initialised with Agents on the bus heading home - every agents has the same\n
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
 * - Agents check the time in the current place, and move to the next place when the time passes a threshhold. \n
 * agents rules specify the next place type to move to and then they go there. The time need not be a multiple of the timestep\n
 * but actual time spent in each palce will in practice end up being some mulitple of the timestep.\n
 * - Structure is included to allow agents to add and remove themsleves from a list of agents in each place\n
 * but at present this is not needed (and the add and remove process is computationally quite expensive). \n
 * Agents just keep a pointer ot a given place, and interaction is implicit (currently only through contamination level).
 */
