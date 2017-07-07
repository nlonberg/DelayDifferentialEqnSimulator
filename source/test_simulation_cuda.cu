#include "arg_parse.hpp"
#include "simulation_cuda.hpp"
#include "analysis.hpp"
#include "csvr_param.hpp"
#include <limits>
#include <iostream>

#define CPUGPU_ALLOC(type, var, ...) \
  type* var##_ptr; \
  check(cudaMallocManaged(&var##_ptr, sizeof(type))); \
  type& var = *(new(var##_ptr) type(__VA_ARGS__))

#define CPUGPU_DELETE(type, var) \
  var.~type(); \
  check(cudaFree(&var));

int main(int argc, char *argv[]) {
    cudaSetDevice(0);

    arg_parse::init(argc, argv);
    
    //setting up model
    CPUGPU_ALLOC(model, m, arg_parse::get<bool>("G", "gradients", false),
        arg_parse::get<bool>("P", "perturb", false));
    
    //setting up param_set
    param_set ps;

    csvr_param csvrp(arg_parse::get<string>("p", "param-sets", "../models/her_model_2014/param_sets.csv"));
    
    if (csvrp.is_open())
    {
        unsigned int set_n = 0;
        while (csvrp.get_next(ps))
        {
            cout << "loaded param_set " << set_n++ << endl;
            CPUGPU_ALLOC(param_set, cudaps);
            cudaps = ps;
            cudaps.printall();
            
            //setting up simulation
            RATETYPE analysis_interval = arg_parse::get<RATETYPE>("a","analysis_interval",0.1);

            CPUGPU_ALLOC(simulation_cuda, s, m, cudaps,
                arg_parse::get<int>("c", "cell-total", 200),
                arg_parse::get<int>("w", "total-width", 50),
                arg_parse::get<RATETYPE>("s", "step-size", 0.01),
                analysis_interval,
                arg_parse::get<RATETYPE>("t", "sim_time", 60) );
       
            // DataLogger dl(&s); 
            s.initialize();

            //BasicAnalysis a(&s);
            //OscillationAnalysis o(&s,analysis_interval,arg_parse::get<RATETYPE>("r","local_range",4),ph1);
            //BasicAnalysis a(&s);
            //run simulation
            s.simulate_cuda();
            //s.print_delay()	
            //o.test();
            //a.test();
            CPUGPU_DELETE(simulation_cuda, s);
            CPUGPU_DELETE(param_set, cudaps);
        }
    }
    CPUGPU_DELETE(model, m);
}
