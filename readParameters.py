#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Sep  7 16:23:44 2021

@author: Mike Bithell
Use a csv file to define sets of values to be used in an experiment.
Read in the base model parameter file as specified for the experiment and write
out a set of parameter files to cover the parameter space with parameters modified
"""
import csv

with open('experiment1.csv') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            if row[0]=='base':
                baseFile=row[1]
            print(f'\t{row[0]} takes values {row[1]} and {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')
    
with open(baseFile, 'r') as reader:
    #read in the entire file to a list, one element per line
    # Note: readlines doesn't trim the line endings - so need to include \n
    parameters = reader.readlines()

#find and change a given parameter
def change(old,new):
    a=next((s for s in parameters if s.startswith(old)), 'missing')
    if a!='missing':
        ik=parameters.index(a)
        parameters[ik]=new
#change parameter - note need to give whole line including termination
change("run.nAgents","run.nAgents=17\n")
with open('otherParameterFile', 'w') as writer:
    # Write out the whole parameter file
     writer.writelines(parameters)
