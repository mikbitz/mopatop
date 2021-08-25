# Simple function to allow iteration over a floating point range
# Note: All arguments are required.
def frange(start, stop, step):
     i = start
     while i < stop:
         yield i
         i += step

#The solutions should all scale so that the population fractions
#evolve in the same way for a given r/N and B, starting from the same I0/N
#Total infected fraction should begin to take off at b/r = BN/r ~ 1
import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import odeint
plt.ion()

#--------------------------------------------------
def f(y, t):
        Si = y[0]
        Ii = y[1]
        Ri = y[2]
        # the model equations

        f0 = - b*Si*Ii
        f1 =   b*Si*Ii - r*Ii
        f2 =             r*Ii
        return [f0, f1, f2]
#--------------------------------------------------
b=0.3     #rate per unit time - for fixed b and r solutions are the same indep. of N (Keeling and Rohani p.18)
          #decrease b shifts infection peak to later times, essentially independent of r once R0 above about 2
r  = 0.1  # recovery rate per unit time - peak height depends on r and b  - curves over as a fn. of R0 - looks logarithmicish
          #total pop. actually as we are using b and r appropriate to fractional values, we have N=1. and S/R/I < 1.
N=1.
B  = b/N # per capita rate for absolute sized populations (rather than fractions) : not used here as all solutions are the same at
         # given b,r and infec0, irrespective of N
#initial infected
infec0=0.1
# solve the system dy/dt = f(y, t)
# initial conditions
S0 = N-infec0             # initial population
I0 = infec0               # initial infected
R0 = 0                  # initial recovered population
y0 = [S0, I0, R0]       # initial condition vector

t  = np.linspace(0.,60, 50)   # time grid

# solve the DEs
soln = odeint(f, y0, t)
St = soln[:, 0]
It = soln[:, 1]
Rt = soln[:, 2]
#for j in range (0,499):
#    print S[j], I[j], R[j]
#print max(I)
#for j in range (0,499):
#    if (I[j]==max(I)): print j

# plot results
plt.figure()
plt.plot(t, St, label='Susceptible')
plt.plot(t, It, label='Infected')
plt.plot(t, Rt, label='Recovered')

plt.xlabel('Days from outbreak')
plt.ylabel('Population')
plt.title('Disease - No Init. Recovered Pop.')
plt.legend(loc=0)
#look for the threshhold - should be where b/r=1.

#Sweep parameter space in recovery rate r and infection rate b
for r in frange (0.1,0.4,0.1):
    for b in frange(0.1,0.4,0.1):
        #create a formatted string for the file name 
        #otherwise the values for e.g. b might look like 0.40000000003
        fl=open('diffEQ_r_{:1.2}_b_{:1.2}.txt'.format(r,b),"w")
        #r = b/r0
        soln = odeint(f, y0, t)
        St = soln[:, 0]
        It = soln[:, 1]
        Rt = soln[:, 2]
        fl.write('Suscetible,Infected,Recovered\n')
        for j in range (0,50):
            #create a string for f.write using the output
            fl.write(f'{St[j]},{It[j]},{Rt[j]}\n')

        fl.close()

