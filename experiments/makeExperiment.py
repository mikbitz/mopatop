#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##@file makeExperiment.py
#@brief Create csv files to define sets of values to be used in an experiment.
#@details Read in the file that lists all the experiments, then 
#create a new row for the new experiment, including default path and specification
#file name. Write this to the experiment database. Then create the experiment
#directories, and use the example experiment to create a default parameter file
#and specification file (which says how the parameters will be varied during the 
#experiment).
#@author: Mike Bithell
#Created on Mon Sep  20 15:00:44 2021
import pandas as pd
import os

##@brief experiment name
#@details Change this value to the name of the new experiment to be created
Name = 'stochasticVariation'
##@brief change to reflect version of the model
#@details its expected that each expereriment is intended to run with a particular model version\n
#which will be recorded in the experiment database
modelVersion='0.4'
##@brief A string briefly giving experiment purpose
#@details Change this to describe the experiment. More detail can be added in the parameter file and experiment README file
purpose='Testing how randomseeds and numerbs of agents affect the output'
##@brief default name of the specification file
#This will be the name in the experiment database
spec='base_'+Name+'.csv'
##@brief directory from which to find the experiments sub-directory
#@details the path here is relative to the place from which this cide is run
root="../"
##@brief directory containing experiment specifications and their output
#@details defaults to experiments directory under the root
experimentDirectory='experiments/'
##@brief store the while path for later use
path=root+experimentDirectory
##@brief set the name of the experiment database
#@details By default jsut a csv file listing of known experiments, one per row
databaseFile='ExperimentDatabase.csv'
##@brief read in the database and store the contents
#@details the values are stored in a pandas dataframe
experiments=pd.read_csv(path+databaseFile)
##@brief construct new Experiment row
#@details this is a dictionary, with keys equal to the dataframe column names 
newxp={'Name':Name, 'modelVersion':modelVersion, 'baseDirectory':experimentDirectory+Name, 'specificationFile':spec, 'purpose':purpose}
##@brief add new row into new dataframe
#@details the row will be added at the end of the current set of experiments.\n
#Note this is not checked for duplicate names!
#Values will get overwritten into the original database file
newE=experiments.append(newxp,ignore_index=True)
#write dataframe into the original database file
newE.to_csv(path+databaseFile,index=False)
#create a directory fo the new experiment
os.system("mkdir "+path+Name)
#make directory to hold parameter files for the experiment
os.system("mkdir "+path+Name+"/parameterSets")
#make directory to hold informational messages from each model run
os.system("mkdir "+path+Name+"/RunFiles")
#copy in the example speficication file to the new name
os.system("cp "+path+"example/baseExample.csv "+path+Name+"/"+spec)
#copy in the example parameter file
os.system("cp "+path+"example/exampleBaseParameters "+path+Name+"/"+Name+"BaseParameters")
#copy in the README
os.system("cp "+path+"example/README "+path+Name+"/README")
##@brief a dataframe to hold the new experiment
#@details Use the default file copied in from the example, and modify for the new experiment
thisExperiment=pd.read_csv(path+Name+"/"+spec)
#correct the experiment name 
#first value in the first row after the header row
thisExperiment.iat[0,0]=Name
#correct the base parameter file name, second value in the first row
thisExperiment.iat[0,1]=Name+"BaseParameters"
#write out the spec file
thisExperiment.to_csv(path+Name+"/"+spec,index=False)
#print message so that user knows what to do next...
print("Experiment specification "+Name+" created in "+path+Name)
print("This experiment has been added as a new row in "+path+databaseFile )
print("Now edit "+path+Name+"/"+Name+"BaseParameters "+ "to create the default set up")
print("Then edit "+path+Name+"/"+spec+" to change the parameters")
print("Also correct the model description in "+path+Name+"/README")
print("Then you can run the experiment (or create a parameter file set) using "+path+"runExperiment.py"  )
