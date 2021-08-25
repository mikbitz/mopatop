#ifndef TIMESTEP_H_INCLUDED
#define TIMESTEP_H_INCLUDED

class timeStep{
    static double years;
    static double months;
    static double days;
    static double hours;
    static double minutes;
    static double seconds;
    static double dt;
    std::string units;
public:
    timeStep(){
        units="hours";
        years=24*30*365;
        months=24*30;
        days=24;
        hours=1;
        minutes=1./60;
        seconds=1./3600;
        dt=1;
    }
    timeStep(parameterSettings& p){
        units     = p.get("timesStep.units");
        //years     = p.get("timesStep.years");
        //months    = p.get("timesStep.months");
        //hours     = p.get("timesStep.hours");
        //minutes   = p.get("timesStep.minutes");
        //seconds   = p.get("timesStep.seconds");
        dt        = p.get<int>("timesStep.dt");
    }
};
//setup dt to be 1 hour if nothing else is specified
double timeStep::years=24*30*365;
double timeStep::months=24*30;
double timeStep::days=24;
double timeStep::hours=1;
double timeStep::minutes=1/60;
double timeStep::seconds=1/3600;
double timeStep::dt=1;
#endif // TIMESTEP_H_INCLUDED
