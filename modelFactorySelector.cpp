#include "modelFactory.h"
#include "modelFactorySelector.h"
modelFactory&  modelFactorySelector::select(std::string name){
    modelFactory* F=nullptr;
    if (name=="simpleOnePlace")F=new simpleOnePlaceFactory();
    if (name=="simpleMobile")  F=new simpleMobileFactory();
    if (name=="flying")  F=new flyingFactory();
    if (F==nullptr)std::cout<<"Name "<<name<<" not recognised in modelFactorySelector"<<std::endl;
    assert(F!=nullptr);
    return *F;
}
