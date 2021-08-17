#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Aug 17 15:37:56 2021

@author: moke
"""

import pandas as pd
import matplotlib
matplotlib.use('Qt5Agg')
from matplotlib import pyplot as plt
fig=plt.figure()
fig.set_size_inches(3.5, 4., forward=True)

ax=fig.add_axes()

#plt.subplots_adjust(left=0.1, right=0.9, bottom=0.1, top=0.9)

df = pd.read_csv('release/diseaseSummary.csv')
plt.plot(df['step']/24,df['susceptible'])
plt.plot(df['step']/24,df['infected'])
plt.plot(df['step']/24,df['recovered'])
plt.xlabel('Time (days)')
plt.ylabel('Number of agents')
plt.title('Simple disease')
plt.legend()
fig.show()