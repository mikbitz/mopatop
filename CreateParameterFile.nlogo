to resetToDefaults
  set nAgents 10
  set nSteps 1000
  set nThreads 1
  set randomSeed 0
  set randomIncrement 57
  set nRepeats 1
  set schedule "stationary"
  set modelType "simpleOnePlace"
  set timeStepUnits "hours"
  set timeStep 1
  set runNumber -1
  set runPrefix 10000
  set experimentName "default"
  set experimentDescription "default parameters used"
  set outputDirectory "./output"
  set outputFile "diseaseSummary"
  set parameterFileName "parameterFile"
  set cleanContamination "true"
  set fractionalDecrement 0.9
  set recoveryRate 0.008
  set deathRate 0.
  set infectionLoad 0.01
  set initialNumberInfected 1
end
to writeParameters
  if file-exists? parameterFileName [ error "Attempt to overwrite existing file!" ]
  file-open parameterFileName
  file-print "#Model parameters"
  file-print "#lines starting with any amount of white space, followed by # are comments"
  file-print "#Lines with no = will also be skipped"
  file-print "#Format is label=value on each line"
  file-print "#Label is generally <category>.name"
  file-print "#These values will be output alphabetically to a filecalled RunParameters"
  file-print "#So <Category> helps to group parameters together in this file"
  file-print ""
  file-print "#Parameters that exist in the code but not set here will assume default values "
  file-print "#see parameters.h - add new parameters to the setDefaults method before trying to set them here"
  file-print ""
  file-print "#Attempts to set parameters not already defined in the code will cause code execution to halt"
  file-print ""
  file-print "#-------------------------------"
  file-print "#run control"
  file-print "#-------------------------------"
  file-print "#Number of agents to create - integer"
  file-print word "run.nAgents=" nAgents
  file-print ""
  file-print "#Number of steps to run for - integer"
  file-print word "run.nSteps=" nSteps
  file-print ""
  file-print "#number of OMP threads to use - integer"
  file-print "#Increase the number here if using openmp to parallelise any loops."
  file-print "#Note number of threads needs to be <= to number of cores/threads supported on the local machine"
  file-print word "run.nThreads=" nThreads
  file-print ""
  file-print "#random seed - integer"
  file-print "#change to vary the stochastic parts of the model"
  file-print "#runs with the same seed should produce the same output...although maybe not if nThreads>1"
  file-print word "run.randomSeed=" randomSeed
  file-print ""
  file-print "#how many times to repeat the run with these parameters - integer"
  file-print "#Leave this set to 1 unles you want multiple runs with different random seeds"
  file-print "#If > 1 the same parameters will be used for nRepeats, but with different random seeds starting from"
  file-print "#run.randomSeed above, and then adding run.randomIncrement for each run"
  file-print "#Values <=0 here will get set to 1"
  file-print word "run.nRepeats=" nRepeats
  file-print ""
  file-print "#Add this value to the seed for each successive run - integer"
  file-print "#Note in a multithreaded run run.Nthreads RNG are created each separated by an increment of 1 in the seed"
  file-print word "run.randomIncrement=" randomIncrement
  file-print ""
  file-print "#NB setting repeats to more than 1 will set autmatically set and increase experiment.run.number irrespective of any value set below. "
  file-print ""
  file-print "#-------------------------------"
  file-print "#schedule"
  file-print "#-------------------------------"
  file-print "#pick schedule from either mobile or stationary - string"
  file-print "#if you set simpleMobile below, you probably want mobile here too..."
  file-print "#if you set mobile here and simpleOnePlace below then work,home and transport will all actually be the same place"
  file-print word "schedule.type=" schedule
  file-print ""
  file-print "#-------------------------------"
  file-print "#model"
  file-print "#-------------------------------"
  file-print "#pick model type either simpleMobile or simpleOnePlace - string"
  file-print "#simpleOnePlace puts all agents into a single location, and there they stay."
  file-print word "model.type=" modelType
  file-print ""
  file-print "#-------------------------------"
  file-print "#timestepping"
  file-print "#-------------------------------"
  file-print "#units to use to determine how much real time is represetned by each model timestep"
  file-print "#valid values are years,months,days,hours,minutes or seconds"
  file-print word "timeStep.units=" timeStepUnits
  file-print ""
  file-print "#the actual time duration of each step in the above units - double"
  file-print "#so if this is set to 2 and the above units set to hours, each timestep will represent 2 hours of real time"
  file-print "#note that any rates set in this file will need to take into account this timing "
  file-print word "timeStep.dt=" timeStep
  file-print "#-------------------------------"
  file-print "#output"
  file-print "#-------------------------------"
  file-print "#Unique name for this set of model runs, if desired - string"
  file-print word "experiment.name=" experimentName
  file-print ""
  file-print "#location for the output files - subdirectory will be created called experiment.<name> - string"
  file-print word "experiment.output.directory=" outputDirectory
  file-print ""
  file-print "#short text to outline experiment purpose - string"
  file-print word "experiment.description="  experimentDescription
  file-print ""
  file-print "#run number - allows runs with above name to be grouped into sets (experiments!) - integer"
  file-print "#If this value is set (uncomment line below) oldfiles may be overwritten. The directory for all runs with this file will be fixed,"
  file-print "#unless repeats is setto more then 1 above."
  file-print "#Otherwise new directories will be auto-created for each run starting at 0000, and incrementing automatically"
  ifelse runNumber != -1 [
      file-print word "#experiment.run.number=" runNumber
  ][
      file-print "#experiment.run.number=0000"
  ]
  file-print ""
  file-print "#Experiments are assumed to be numbered 0000 to 9999 - integer"
  file-print "#Paths will look like experiment.output.directory/experiment.name/run_0000 etc."
  file-print "#if more (or fewer) are needed, specify the maximum number of runs here "
  file-print "#to the nearest power of 10 (using e.g. 100 or 1000 - the leading 1 will be removed - so don't just put 1!)"
  file-print "#if you exceed this number, directory names will still increment by run number, but they won't be all a nice tidy length"
  ifelse runPrefix != 10000 [
      file-print word "#experiment.run.prefix=" runPrefix
  ][
      file-print "#experiment.run.prefix=10000"
  ]
  file-print ""
  file-print "#name of the output file - string"
  file-print word "outputFile=" outputFile
  file-print ""
  file-print "#-------------------------------"
  file-print "#disease"
  file-print "#-------------------------------"
  file-print "#Rate *PER HOUR* - double"
  file-print "#In the simplest case recovery in any timestep will happen if a random number drawn from (0,1) is less than this value"
  file-print word "disease.simplistic.recoveryRate=" recoveryRate
  file-print ""
  file-print "#Rate *PER HOUR* - double"
  file-print "#an infected individual will add this much contamination to any place every timestep"
  file-print word "disease.simplistic.deathRate=" deathRate
  file-print ""
  file-print "#Rate *PER HOUR* - double"
  file-print "#an infected individual will add this much contamination to any place every timestep"
  file-print word "disease.simplistic.infectionShedLoad=" infectionLoad
  file-print ""
  file-print "#how many (randomly allocated) agents have the disease at the start of the run"
  file-print word "disease.simplistic.initialNumberInfected=" initialNumberInfected
  file-print ""
  file-print "#Rate *PER HOUR* - double"
  file-print "#any place contaminated with disease will lose contamination exponentially at this rate"
  file-print word "places.disease.simplistic.fractionalDecrement=" fractionalDecrement
  file-print ""
  file-print "#if this flag is set to true any contamination in a places gets removed at the start of each timestep"
  file-print word "places.cleanContamination=" cleanContamination

  file-close
end
@#$#@#$#@
GRAPHICS-WINDOW
869
20
952
104
-1
-1
2.3
1
10
1
1
1
0
1
1
1
-16
16
-16
16
0
0
1
ticks
30.0

INPUTBOX
62
36
145
96
nAgents
10.0
1
0
Number

INPUTBOX
62
99
146
159
nSteps
1000.0
1
0
Number

INPUTBOX
63
161
146
221
nThreads
1.0
1
0
Number

INPUTBOX
64
285
146
345
randomSeed
0.0
1
0
Number

INPUTBOX
64
223
146
283
nRepeats
1.0
1
0
Number

CHOOSER
191
38
311
83
schedule
schedule
"stationary" "mobile"
0

CHOOSER
191
87
308
132
modelType
modelType
"simpleOnePlace" "simpleMobile"
0

CHOOSER
191
135
312
180
timeStepUnits
timeStepUnits
"hours" "minutes" "seconds" "days" "months" "years"
0

INPUTBOX
191
184
317
244
timeStep
1.0
1
0
Number

INPUTBOX
191
267
316
327
experimentName
default
1
0
String

INPUTBOX
192
330
318
390
outputDirectory
./output
1
0
String

INPUTBOX
321
266
512
395
experimentDescription
default parameters used
1
1
String

INPUTBOX
192
453
320
513
runNumber
-1.0
1
0
Number

INPUTBOX
192
517
325
577
runPrefix
10000.0
1
0
Number

INPUTBOX
192
391
320
451
outputFile
diseaseSummary
1
0
String

BUTTON
582
37
711
70
NIL
resetToDefaults
NIL
1
T
OBSERVER
NIL
NIL
NIL
NIL
1

INPUTBOX
582
113
716
173
parameterFileName
parameterFile
1
0
String

INPUTBOX
331
35
422
95
recoveryRate
0.008
1
0
Number

INPUTBOX
332
97
422
157
deathRate
0.0
1
0
Number

INPUTBOX
332
158
423
218
infectionLoad
0.01
1
0
Number

INPUTBOX
428
157
535
217
initialNumberInfected
1.0
1
0
Number

CHOOSER
429
37
553
82
cleanContamination
cleanContamination
"true" "false"
0

INPUTBOX
429
95
552
155
fractionalDecrement
0.9
1
0
Number

INPUTBOX
64
350
164
410
randomIncrement
57.0
1
0
Number

BUTTON
581
74
715
107
NIL
writeParameters
NIL
1
T
OBSERVER
NIL
NIL
NIL
NIL
1

TEXTBOX
333
20
483
38
Disease Settings
12
0.0
1

TEXTBOX
63
20
213
38
Run Control
12
0.0
1

TEXTBOX
191
20
341
38
Model settings
12
0.0
1

TEXTBOX
193
250
343
268
Output
12
0.0
1

@#$#@#$#@
## WHAT IS IT?

This is a simple generator of parameter files for mopatop

## HOW IT WORKS
Click the defaults button to tsee the deafult settings. Change as needed then click writeParameters. A file with the name given by "parameterFilename" will be generated. 

## HOW TO USE IT

(how to use the model, including a description of each of the items in the Interface tab)

## THINGS TO NOTICE

(suggested things for the user to notice while running the model)

## THINGS TO TRY

(suggested things for the user to try to do (move sliders, switches, etc.) with the model)

## EXTENDING THE MODEL

(suggested things to add or change in the Code tab to make the model more complicated, detailed, accurate, etc.)

## NETLOGO FEATURES

(interesting or unusual features of NetLogo that the model uses, particularly in the Code tab; or where workarounds were needed for missing features)

## RELATED MODELS

(models in the NetLogo Models Library and elsewhere which are of related interest)

## CREDITS AND REFERENCES

(a reference to the model's URL on the web if it has one, as well as any other necessary credits, citations, and links)
@#$#@#$#@
default
true
0
Polygon -7500403 true true 150 5 40 250 150 205 260 250

airplane
true
0
Polygon -7500403 true true 150 0 135 15 120 60 120 105 15 165 15 195 120 180 135 240 105 270 120 285 150 270 180 285 210 270 165 240 180 180 285 195 285 165 180 105 180 60 165 15

arrow
true
0
Polygon -7500403 true true 150 0 0 150 105 150 105 293 195 293 195 150 300 150

box
false
0
Polygon -7500403 true true 150 285 285 225 285 75 150 135
Polygon -7500403 true true 150 135 15 75 150 15 285 75
Polygon -7500403 true true 15 75 15 225 150 285 150 135
Line -16777216 false 150 285 150 135
Line -16777216 false 150 135 15 75
Line -16777216 false 150 135 285 75

bug
true
0
Circle -7500403 true true 96 182 108
Circle -7500403 true true 110 127 80
Circle -7500403 true true 110 75 80
Line -7500403 true 150 100 80 30
Line -7500403 true 150 100 220 30

butterfly
true
0
Polygon -7500403 true true 150 165 209 199 225 225 225 255 195 270 165 255 150 240
Polygon -7500403 true true 150 165 89 198 75 225 75 255 105 270 135 255 150 240
Polygon -7500403 true true 139 148 100 105 55 90 25 90 10 105 10 135 25 180 40 195 85 194 139 163
Polygon -7500403 true true 162 150 200 105 245 90 275 90 290 105 290 135 275 180 260 195 215 195 162 165
Polygon -16777216 true false 150 255 135 225 120 150 135 120 150 105 165 120 180 150 165 225
Circle -16777216 true false 135 90 30
Line -16777216 false 150 105 195 60
Line -16777216 false 150 105 105 60

car
false
0
Polygon -7500403 true true 300 180 279 164 261 144 240 135 226 132 213 106 203 84 185 63 159 50 135 50 75 60 0 150 0 165 0 225 300 225 300 180
Circle -16777216 true false 180 180 90
Circle -16777216 true false 30 180 90
Polygon -16777216 true false 162 80 132 78 134 135 209 135 194 105 189 96 180 89
Circle -7500403 true true 47 195 58
Circle -7500403 true true 195 195 58

circle
false
0
Circle -7500403 true true 0 0 300

circle 2
false
0
Circle -7500403 true true 0 0 300
Circle -16777216 true false 30 30 240

cow
false
0
Polygon -7500403 true true 200 193 197 249 179 249 177 196 166 187 140 189 93 191 78 179 72 211 49 209 48 181 37 149 25 120 25 89 45 72 103 84 179 75 198 76 252 64 272 81 293 103 285 121 255 121 242 118 224 167
Polygon -7500403 true true 73 210 86 251 62 249 48 208
Polygon -7500403 true true 25 114 16 195 9 204 23 213 25 200 39 123

cylinder
false
0
Circle -7500403 true true 0 0 300

dot
false
0
Circle -7500403 true true 90 90 120

face happy
false
0
Circle -7500403 true true 8 8 285
Circle -16777216 true false 60 75 60
Circle -16777216 true false 180 75 60
Polygon -16777216 true false 150 255 90 239 62 213 47 191 67 179 90 203 109 218 150 225 192 218 210 203 227 181 251 194 236 217 212 240

face neutral
false
0
Circle -7500403 true true 8 7 285
Circle -16777216 true false 60 75 60
Circle -16777216 true false 180 75 60
Rectangle -16777216 true false 60 195 240 225

face sad
false
0
Circle -7500403 true true 8 8 285
Circle -16777216 true false 60 75 60
Circle -16777216 true false 180 75 60
Polygon -16777216 true false 150 168 90 184 62 210 47 232 67 244 90 220 109 205 150 198 192 205 210 220 227 242 251 229 236 206 212 183

fish
false
0
Polygon -1 true false 44 131 21 87 15 86 0 120 15 150 0 180 13 214 20 212 45 166
Polygon -1 true false 135 195 119 235 95 218 76 210 46 204 60 165
Polygon -1 true false 75 45 83 77 71 103 86 114 166 78 135 60
Polygon -7500403 true true 30 136 151 77 226 81 280 119 292 146 292 160 287 170 270 195 195 210 151 212 30 166
Circle -16777216 true false 215 106 30

flag
false
0
Rectangle -7500403 true true 60 15 75 300
Polygon -7500403 true true 90 150 270 90 90 30
Line -7500403 true 75 135 90 135
Line -7500403 true 75 45 90 45

flower
false
0
Polygon -10899396 true false 135 120 165 165 180 210 180 240 150 300 165 300 195 240 195 195 165 135
Circle -7500403 true true 85 132 38
Circle -7500403 true true 130 147 38
Circle -7500403 true true 192 85 38
Circle -7500403 true true 85 40 38
Circle -7500403 true true 177 40 38
Circle -7500403 true true 177 132 38
Circle -7500403 true true 70 85 38
Circle -7500403 true true 130 25 38
Circle -7500403 true true 96 51 108
Circle -16777216 true false 113 68 74
Polygon -10899396 true false 189 233 219 188 249 173 279 188 234 218
Polygon -10899396 true false 180 255 150 210 105 210 75 240 135 240

house
false
0
Rectangle -7500403 true true 45 120 255 285
Rectangle -16777216 true false 120 210 180 285
Polygon -7500403 true true 15 120 150 15 285 120
Line -16777216 false 30 120 270 120

leaf
false
0
Polygon -7500403 true true 150 210 135 195 120 210 60 210 30 195 60 180 60 165 15 135 30 120 15 105 40 104 45 90 60 90 90 105 105 120 120 120 105 60 120 60 135 30 150 15 165 30 180 60 195 60 180 120 195 120 210 105 240 90 255 90 263 104 285 105 270 120 285 135 240 165 240 180 270 195 240 210 180 210 165 195
Polygon -7500403 true true 135 195 135 240 120 255 105 255 105 285 135 285 165 240 165 195

line
true
0
Line -7500403 true 150 0 150 300

line half
true
0
Line -7500403 true 150 0 150 150

pentagon
false
0
Polygon -7500403 true true 150 15 15 120 60 285 240 285 285 120

person
false
0
Circle -7500403 true true 110 5 80
Polygon -7500403 true true 105 90 120 195 90 285 105 300 135 300 150 225 165 300 195 300 210 285 180 195 195 90
Rectangle -7500403 true true 127 79 172 94
Polygon -7500403 true true 195 90 240 150 225 180 165 105
Polygon -7500403 true true 105 90 60 150 75 180 135 105

plant
false
0
Rectangle -7500403 true true 135 90 165 300
Polygon -7500403 true true 135 255 90 210 45 195 75 255 135 285
Polygon -7500403 true true 165 255 210 210 255 195 225 255 165 285
Polygon -7500403 true true 135 180 90 135 45 120 75 180 135 210
Polygon -7500403 true true 165 180 165 210 225 180 255 120 210 135
Polygon -7500403 true true 135 105 90 60 45 45 75 105 135 135
Polygon -7500403 true true 165 105 165 135 225 105 255 45 210 60
Polygon -7500403 true true 135 90 120 45 150 15 180 45 165 90

sheep
false
15
Circle -1 true true 203 65 88
Circle -1 true true 70 65 162
Circle -1 true true 150 105 120
Polygon -7500403 true false 218 120 240 165 255 165 278 120
Circle -7500403 true false 214 72 67
Rectangle -1 true true 164 223 179 298
Polygon -1 true true 45 285 30 285 30 240 15 195 45 210
Circle -1 true true 3 83 150
Rectangle -1 true true 65 221 80 296
Polygon -1 true true 195 285 210 285 210 240 240 210 195 210
Polygon -7500403 true false 276 85 285 105 302 99 294 83
Polygon -7500403 true false 219 85 210 105 193 99 201 83

square
false
0
Rectangle -7500403 true true 30 30 270 270

square 2
false
0
Rectangle -7500403 true true 30 30 270 270
Rectangle -16777216 true false 60 60 240 240

star
false
0
Polygon -7500403 true true 151 1 185 108 298 108 207 175 242 282 151 216 59 282 94 175 3 108 116 108

target
false
0
Circle -7500403 true true 0 0 300
Circle -16777216 true false 30 30 240
Circle -7500403 true true 60 60 180
Circle -16777216 true false 90 90 120
Circle -7500403 true true 120 120 60

tree
false
0
Circle -7500403 true true 118 3 94
Rectangle -6459832 true false 120 195 180 300
Circle -7500403 true true 65 21 108
Circle -7500403 true true 116 41 127
Circle -7500403 true true 45 90 120
Circle -7500403 true true 104 74 152

triangle
false
0
Polygon -7500403 true true 150 30 15 255 285 255

triangle 2
false
0
Polygon -7500403 true true 150 30 15 255 285 255
Polygon -16777216 true false 151 99 225 223 75 224

truck
false
0
Rectangle -7500403 true true 4 45 195 187
Polygon -7500403 true true 296 193 296 150 259 134 244 104 208 104 207 194
Rectangle -1 true false 195 60 195 105
Polygon -16777216 true false 238 112 252 141 219 141 218 112
Circle -16777216 true false 234 174 42
Rectangle -7500403 true true 181 185 214 194
Circle -16777216 true false 144 174 42
Circle -16777216 true false 24 174 42
Circle -7500403 false true 24 174 42
Circle -7500403 false true 144 174 42
Circle -7500403 false true 234 174 42

turtle
true
0
Polygon -10899396 true false 215 204 240 233 246 254 228 266 215 252 193 210
Polygon -10899396 true false 195 90 225 75 245 75 260 89 269 108 261 124 240 105 225 105 210 105
Polygon -10899396 true false 105 90 75 75 55 75 40 89 31 108 39 124 60 105 75 105 90 105
Polygon -10899396 true false 132 85 134 64 107 51 108 17 150 2 192 18 192 52 169 65 172 87
Polygon -10899396 true false 85 204 60 233 54 254 72 266 85 252 107 210
Polygon -7500403 true true 119 75 179 75 209 101 224 135 220 225 175 261 128 261 81 224 74 135 88 99

wheel
false
0
Circle -7500403 true true 3 3 294
Circle -16777216 true false 30 30 240
Line -7500403 true 150 285 150 15
Line -7500403 true 15 150 285 150
Circle -7500403 true true 120 120 60
Line -7500403 true 216 40 79 269
Line -7500403 true 40 84 269 221
Line -7500403 true 40 216 269 79
Line -7500403 true 84 40 221 269

wolf
false
0
Polygon -16777216 true false 253 133 245 131 245 133
Polygon -7500403 true true 2 194 13 197 30 191 38 193 38 205 20 226 20 257 27 265 38 266 40 260 31 253 31 230 60 206 68 198 75 209 66 228 65 243 82 261 84 268 100 267 103 261 77 239 79 231 100 207 98 196 119 201 143 202 160 195 166 210 172 213 173 238 167 251 160 248 154 265 169 264 178 247 186 240 198 260 200 271 217 271 219 262 207 258 195 230 192 198 210 184 227 164 242 144 259 145 284 151 277 141 293 140 299 134 297 127 273 119 270 105
Polygon -7500403 true true -1 195 14 180 36 166 40 153 53 140 82 131 134 133 159 126 188 115 227 108 236 102 238 98 268 86 269 92 281 87 269 103 269 113

x
false
0
Polygon -7500403 true true 270 75 225 30 30 225 75 270
Polygon -7500403 true true 30 75 75 30 270 225 225 270
@#$#@#$#@
NetLogo 6.1.1
@#$#@#$#@
@#$#@#$#@
@#$#@#$#@
@#$#@#$#@
@#$#@#$#@
default
0.0
-0.2 0 0.0 1.0
0.0 1 1.0 0.0
0.2 0 0.0 1.0
link direction
true
0
Line -7500403 true 150 150 90 180
Line -7500403 true 150 150 210 180
@#$#@#$#@
0
@#$#@#$#@
