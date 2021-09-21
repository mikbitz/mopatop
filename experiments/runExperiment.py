#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Sep  7 16:23:44 2021

@author: Mike Bithell
@brief Use csv files to define sets of values to be used in an experiment.
@details Read in the file that lists all the experiments, then 
select on experiment to run, and use the base model parameter file as specified
for the experiment to write out a set of parameter files to cover the parameter
space. The expectation is that all parameters to be changed will be given in
the header row of the experiment specification file, starting at column 2
(columns 0 and 1 reserved for experiment name and parameter file name), with one
row for every parameter set to be used (every parameter to be changed must
have an entry in every row). This script can either directly run the model now
(given the name of an executable), or else store parameter files for later
(useful for hpc, for example). Note that all the paths for the experiment to 
be run need to be set up ahead of time, consistent with the base parameter file.
"""
import pandas as pd
import os
import sys
from datetime import datetime

#Name of the experiment of the experiment to run
experimentName='endTimeTest'
#if set to True, run the model now! otherwise output a set of parameterFiles
directRun=True
#needed if running the model now - the name of the executable to run
#assumed to live in root directory as below
execname='agentModel'
#directory from which to find the experiments sub-directory
root="../"
#file with listing of known experiments
databaseFile='ExperimentDatabase.csv'
#directory containing experiment specifications and their output
experimentDirectory='experiments/'
#read in the experiment database
experiments=pd.read_csv(root+experimentDirectory+databaseFile)
#row of the experiment database giving the experiment to run
expE=experiments.index[experiments['Name']==experimentName]
#check experiment exists
if (len(expE)==0):
   sys.exit("Experiment "+experimentName+" not in database file "+root+experimentDirectory+databaseFile) 
#extract the integer value from the list - there should only be one!
if (len(expE)>1):
   sys.exit("Experiment "+experimentName+" occurs multiple times in database file "+root+experimentDirectory+databaseFile)
experimentToRun=expE[0]
#path to the directory for this experiment
path=root+experiments['baseDirectory'][experimentToRun]
#file speciying experiment name and parameters
spec=path+'/'+experiments['specificationFile'][experimentToRun]
#read the specification file, which has the parameter sets
#and the name of the base parameter file. which will be modified for runs
thisExperiment=pd.read_csv(spec)
#get the parameter file name
baseParameterFile=path+'/'+thisExperiment['baseParameterFile'][0] 

#read the parameter file that forms the basis for the experiment
with open(baseParameterFile, 'r') as reader:
    #read in the entire file to a list, one element per line
    # Note: readlines doesn't trim the line endings - so need to include \n
    parameters = reader.readlines()

#function find and change a given parameter
def change(old,new):
    a=next((s for s in parameters if s.startswith(old)), 'missing')
    if a!='missing':
        ik=parameters.index(a)
        parameters[ik]=new
    else:
        print("missing parameter! "+old)
        sys.exit("terminated")
        
#The path needs a little manipulation to keep runs separate from specification
#This formulation makes a directory called "runs" in the directory named
#for the experiment.
change("experiment.output.directory","experiment.output.directory="+path+"\n")
change("experiment.name","experiment.name=runs\n")    
   
#get the number of columns and rows for the parameters
rows=thisExperiment.shape[0]
cols=thisExperiment.shape[1]
#loop over the parameter sets, 1 per row
for parameterSet in range(rows):
    #loop over the parameters to be changed from the base parameter file
    for m in range(cols-2):
        param=thisExperiment.columns[m+2]
        newValue=str(thisExperiment.loc[parameterSet,param])
        #change parameters - note need to give whole line including termination
        change(param,param+"="+newValue+"\n")


    if (directRun):
        #write a temporary file for the run
        with open('temporaryParameterFile', 'w') as writer:
            # Write out the whole parameter file in one go
            writer.writelines(parameters)
        #Directly run the model!
        #format a current time string
        now=datetime.now().strftime("%d-%m-%Y_%H:%M:%S")
        #output the model trace to a file called RunFile with the current date/time
        #add the parameterSet number in case the time is the same!
        print("Running model set:"+str(parameterSet))
        os.system(root+execname+" temporaryParameterFile >"+path+"/RunFile"+str(parameterSet)+"_"+now)
        print("Removing temporary parameter file")
        os.system("rm temporaryParameterFile")

    else:
        #write one parameter file for each set
        print("Writing parameter file for set "+str(parameterSet)+ " to "+path+'/parameterSets/')
        with open(path+'/parameterSets/parameterFile'+str(parameterSet), 'w') as writer:
            # Write out the whole parameter file in one go
            writer.writelines(parameters)