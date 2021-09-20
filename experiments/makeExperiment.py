#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Sep  20 15:00:44 2021

@author: Mike Bithell
@brief Create csv files to define sets of values to be used in an experiment.
@details Read in the file that lists all the experiments, then 
create a new row for the new experiment, including default path and specification
file name. Write this to the experiment database. Then create the experiment
directories, and use the example experiment to create a default parameter file
and specification file (which says how the parameters will be varied during the 
experiment).
"""
import pandas as pd
import os

#experiment name
Name='smallStochastic'
#which version of the model is intended
modelVersion='0.4'
#A string briefly giving experiment purpose
purpose='Test Stochastic Convergence of Simple One Place Model'
#default name of the specification file
spec='base_'+Name+'.csv'
#directory from which to find the experiments sub-directory
root="../"
#directory containing experiment specifications and their output
experimentDirectory='experiments/'
path=root+experimentDirectory
#file with listing of known experiments
databaseFile='ExperimentDatabase.csv'
#read in the experiment database
experiments=pd.read_csv(path+databaseFile)
#constrcut new Experiment row
newxp={'Name':Name, 'modelVersion':modelVersion, 'baseDirectory':experimentDirectory+Name, 'specificationFile':spec, 'purpose':purpose}
#add new row into new dataframe
newE=experiments.append(newxp,ignore_index=True)
#write dataframe into the database file
newE.to_csv(path+databaseFile,index=False)
#create a directory fo the new experiment
os.system("mkdir "+path+Name)
#make directory to hold parameter files for the experiment
os.system("mkdir "+path+Name+"/parameterSets")
#copy in the example speficication file to the new name
os.system("cp "+path+"example/baseExample.csv "+path+Name+"/"+spec)
#copy in the example parameter file
os.system("cp "+path+"example/exampleBaseParameters "+path+Name+"/"+Name+"BaseParameters")
#open the new experiment specifcation
thisExperiment=pd.read_csv(path+Name+"/"+spec)
#correct the experiment name and the base parameter file name
thisExperiment.iat[0,0]=Name
thisExperiment.iat[0,1]=Name+"BaseParameters"
#write out the spec file
thisExperiment.to_csv(path+Name+"/"+spec,index=False)
#print message so that user knows what to do next...
print("Experiment specification "+Name+" created in "+path+Name)
print("This experiment has been added as a new row in "+path+databaseFile )
print("Now edit "+path+Name+"/"+Name+"BaseParameters "+ "to create the default set up")
print("Then edit "+path+Name+"/"+spec+" to change the parameters")
print("Then you can run the experiment (or create a parameter file set) using "+path+"runExperiment.py"  )