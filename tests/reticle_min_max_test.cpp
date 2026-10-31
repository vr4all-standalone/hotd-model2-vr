#include "../native/reticle_size.h"
#include "../native/settings_io.h"
#include <cassert>
#include <cmath>
#include <sstream>
using namespace hotd_model2;
static bool near(double a,double b){return std::abs(a-b)<1e-5;}
int main(){
    Settings s;
    assert(near(s.reticle_min_size,.22)&&near(s.reticle_max_size,.75));
    assert(near(reticle_size_reference(1,1.8,reticle_size_for_mode(s)),1.35));
    s.reticle_min_size=.4f;s.reticle_max_size=1.5f;
    assert(near(reticle_size_reference(1,1.8,reticle_size_for_mode(s)),2.7));
    std::ostringstream output;write_settings(output,s);std::istringstream input(output.str());
    auto saved=read_settings(input);
    assert(near(saved.reticle_min_size,.4)&&near(saved.reticle_max_size,1.5));
    std::istringstream bad("reticle_max_size=nan\n");
    assert(near(read_settings(bad).reticle_max_size,.75));
    for(float maximum:{.25f,.75f,1.f,3.f}){
        s.reticle_max_size=maximum;std::ostringstream text;write_settings(text,s);std::istringstream reload(text.str());
        assert(near(read_settings(reload).reticle_max_size,maximum));
    }
    for(const char* invalid:{"bad","inf","nan",".24","3.01"}){
        std::istringstream text(std::string("reticle_max_size=")+invalid+"\n");assert(near(read_settings(text).reticle_max_size,.75));
    }
    assert(near(reticle_size_settings(NAN,NAN,NAN).maximum_ratio,.75));
}
