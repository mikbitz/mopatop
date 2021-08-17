#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#read in the summary output from a disease run and plot
"""
Created on Tue Aug 17 15:37:56 2021

@author: moke
"""

import pandas as pd
import matplotlib
#This line sets the background plotting system - may vary by platform
matplotlib.use('Qt5Agg')

from matplotlib import pyplot as plt
fig=plt.figure()
fig.set_size_inches(3.5, 4., forward=True)

ax=fig.add_axes()


#use a pandas data frame to read in comma separated output with header row
#expected format is that each row represents one time point, timesteps are hours
df = pd.read_csv('release/diseaseSummary.csv')
#use header titles to split out the various data columns
plt.plot(df['step']/24,df['susceptible'])
plt.plot(df['step']/24,df['infected'])
plt.plot(df['step']/24,df['recovered'])
plt.xlabel('Time (days)')
plt.ylabel('Number of agents')
plt.title('Simple disease')
plt.legend()
fig.show()