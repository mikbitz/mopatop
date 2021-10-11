#!/usr/bin/env python3
# -*- coding: utf-8 -*-
##@file parameterMixer.py
#@brief Define sets of values to be used in an experiment and save them to the specification file.
#@details Read in the file that lists all the experiments, then 
#select one experiment and calculate a set of parameter values to be used.
#Write these to the experiment specification file. The latter file is assumed already to
#have been created with makeExperiment.py and to have the column values set to
#meaningful parameter names, taken from the parameterFile for the run. This code
#then allows parameter sets to be calculated algortihmically - e.g. using
#latin hypercube (random selection without replacement), or full factorial (all
#versus all), and to write them to rows of the specification file ready to
#create runs with runExperiemnt.py
#@author: Mike Bithell
#Created on Tue Sep  7 16:23:44 2021

import pandas as pd
import sys

##Name of the experiment of the experiment to run
experimentName='endTimeTest10_20000'
##directory from which to find the experiments sub-directory
root="/home/moke/testAgent/testAgent/"
##file with listing of known experiments
databaseFile='ExperimentDatabase.csv'
##directory containing experiment specifications and their output
experimentDirectory='experiments/'
##read in the experiment database
experiments=pd.read_csv(root+experimentDirectory+databaseFile)
##row of the experiment database giving the experiment to run
expE=experiments.index[experiments['Name']==experimentName]
#check experiment exists
if (len(expE)==0):
   sys.exit("Experiment "+experimentName+" not in database file "+root+experimentDirectory+databaseFile) 
#extract the integer value from the list - there should only be one!
if (len(expE)>1):
   sys.exit("Experiment "+experimentName+" occurs multiple times in database file "+root+experimentDirectory+databaseFile)
##pick out the experiment
experimentToRun=expE[0]
##path to the directory for this experiment
path=root+experiments['baseDirectory'][experimentToRun]
##file speciying experiment name and parameters
spec=path+'/'+experiments['specificationFile'][experimentToRun]
##read the specification file, which has the parameter sets
#and the name of the base parameter file. which will be modified for runs
thisExperiment=pd.read_csv(spec)
#first value in the first row after the header row
Name=thisExperiment.iat[0,0]
#correct the base parameter file name, second value in the first row
BaseParameters=thisExperiment.iat[0,1]
print (thisExperiment.iat[0,1])
#the expectation is that the first row has the name and baseparameters in it already
#It is also assumed that you already set up the column headings following the above
#to list the parameters you want to vary
##Row to be written to the output
row=0
for Rp in range(10,201,2):
    for recov in range(2,22,2):
        R0=Rp/10
        if (row==0):
            N=Name
            B=BaseParameters
        else:
            N=''
            B=''
        p1=recov/100#recovery rate
        p2=p1*R0/20000#infection rate per capita
        thisExperiment.loc[row]=[N,B,p1,p2]
        row=row+1
thisExperiment.to_csv(spec,index=False)


