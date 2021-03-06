#include "io/arg_parse.hpp"
#include "anlys/oscillation.hpp"
#include "util/color.hpp"
#include "io/csvr_sim.hpp"
#include "io/csvw_sim.hpp"
#include "sim/set.hpp"
#include "model_impl.hpp"
#include <ctime>
#include <iostream>


int main(int argc, char *argv[])
{
    arg_parse::init(argc, argv);
    color::enable(!arg_parse::get<bool>("n", "no-color", 0, false));
    
    if (arg_parse::get<bool>("h", "help", false) || arg_parse::get<bool>("H", "usage", false) || argc == 1)
    {
        // # Display all possible command line arguments with descriptions
        cout << color::set(color::YELLOW) <<
            "[-h | --help | --usage]         " << color::set(color::GREEN) <<
            "Print information about program's various command line arguments." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-n | --no-color]               " << color::set(color::GREEN) <<
            "Disable color in the terminal." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-p | --param-sets]    <string> " << color::set(color::GREEN) <<
            "Relative file location and name of the parameter sets csv. \"../param_sets.csv\", for example." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-g | --gradients]     <string> " << color::set(color::GREEN) <<
            "Enables gradients and specifies the relative file location and name of the gradients csv. \"../param_grad.csv\", for example." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-b | --perturbations] <string> " << color::set(color::GREEN) <<
            "Enables perturbations and specifies the relative file location and name of the perturbations csv. \"../param_pert.csv\", for example." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-b|--perturbations] <RATETYPE> " << color::set(color::GREEN) <<
            "Enables perturbations and specifies a global perturbation factor to be applied to ALL reactions. The [-b | --perturb] flag itself is identical to the <string> version; the program automatically detects whether it is in the format of a file or a RATETYPE." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-e | --data-export]   <string> " << color::set(color::GREEN) <<
            "Relative file location and name of the output of the logged data csv. \"../data_out.csv\", for example." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-i | --data-import]   <string> " << color::set(color::GREEN) <<
            "Relative file location and name of csv data to import into the analyses. \"../data_in.csv\", for example. Using this flag runs only analysis." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-o | --specie-option] <string> " << color::set(color::GREEN) <<
            "Specify which species to output to file and analyze. Not including this argument makes the program by default output/analyze all species. IF MORE THAN ONE BUT NOT ALL SPECIES ARE DESIRED, enclose the argument in quotation marks and seperate the species using commas. For example, \"ph13, mh1, ph113\", including quotation marks. If only one specie is desired, no commas or quotation marks are necessary." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-c | --cell-total]       <int> " << color::set(color::GREEN) <<
            "Total number of cells to simulate." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-w | --total-width]      <int> " << color::set(color::GREEN) <<
            "Width of tissue to simulate. Height is inferred by c/w." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-s | --step-size]   <RATETYPE> " << color::set(color::GREEN) <<
            "Time increment by which the deterministic simulation progresses. USING THIS ARGUMENT IMPLICITLY SWITCHES THE SIMULATION FROM STOCHASTIC TO DETERMINISTIC." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-a | --anlys-intvl] <RATETYPE> " << color::set(color::GREEN) <<
            "Analysis AND file writing interval. How frequently (in units of simulated minutes) data is fetched from simulation for analysis and/or file writing." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-l | --local-range] <RATETYPE> " << color::set(color::GREEN) <<
            "Range in which oscillation features are searched for. USING THIS ARGUMENT IMPLICITLY TOGGLES ANALYSIS." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-r | --rand-seed]        <int> " << color::set(color::GREEN) <<
            "Set the stochastic simulation's random number generator seed." << color::clear() << endl;
        cout << color::set(color::yellow) <<
            "[-t | --time]             <int> " << color::set(color::green) <<
            "Amount of simulated minutes the simulation should execute." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-z | --time-col]        <bool> " << color::set(color::GREEN) <<
            "Toggles whether file output includes a time column. Convenient for making graphs in Excel-like programs, but slows down file writing. Time could be inferred without this column through the row number and the analysis interval." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-u | --time-start]  <RATETYPE> " << color::set(color::GREEN) <<
            "Simulation time at which analysis and file writing should start. Not including this argument defaults the start time to the very beginning." << color::clear() << endl;
        cout << color::set(color::YELLOW) <<
            "[-v | --time-end]    <RATETYPE> " << color::set(color::GREEN) <<
            "Simulation time at which analysis and file writing should end. Not including this argument defaults the end time to the very end." << color::clear() << endl;
        cout << color::set(color::yellow) <<
            "[-x | --cell-start]             <int> " << color::set(color::green) <<
            "The first column of cells that the analyzer and file writer should start paying attention at." << color::clear() << endl;
        cout << color::set(color::yellow) <<
            "[-y | --cell-end]             <int> " << color::set(color::green) <<
            "The last column of cells that the analyzer and file writer should bother looking at." << color::clear() << endl;

    }
    else
    {
        // Allow for analysis and various configs to be optional
        // *_end vars can't be determined till later because their default
        //   values should be sim_time (i.e. total time) and cell_total
        RATETYPE local_range, anlys_intvl,
            time_start = arg_parse::get<RATETYPE>("u", "time-start", 0.0),
            time_end;
        const bool
            do_anlys = 
                arg_parse::get<RATETYPE>("l", "local-range", &local_range, false),
            do_time_col = arg_parse::get<bool>("z", "time-col", false);
        int cell_start = arg_parse::get<int>("x", "cell-start", 0),
            cell_end, cell_total;

        // If there's no -o argument, specie_option will default to all species
        specie_vec specie_option;
        arg_parse::get<specie_vec>("o", "specie-option", &specie_option, false);
        

        // Figure out whether we're importing or actually simulating
        string data_import;
        if (arg_parse::get<string>("i", "data-import", &data_import, false))
        {
            csvr_sim csvrs(data_import, specie_option);
            time_end = arg_parse::get<RATETYPE>("v", "time-end", 0/*getTimeTotal*/);
            cell_end = arg_parse::get<int>("y", "cell-end", 0/*getCellTotal*/);
            anlys_intvl =
                arg_parse::get<RATETYPE>("a", "anlys-intvl", 0/*getAnlysIntvl*/);

            // A good csvw debugging test
            // csvw_sim csvws("data_in_copy_out.csv",
            //        anlys_intvl, cell_total, specie_option, &csvrs);

            // Analysis optional
            if (do_anlys)
            {
                // Prepare analyses
                //BasicAnalysis ba(&csvrs);
                OscillationAnalysis *oa[specie_option.size()];
                for (unsigned int i=0; i<specie_option.size(); i++)
                {
                    // TODO have observable getMin and getMax funcs for time and cell
                    oa[i] = new OscillationAnalysis(&csvrs,
                            anlys_intvl, local_range, specie_option.at(i),
                            cell_start, cell_end, time_start, time_end);
                }

                // Emulate a simulation
                csvrs.run();
                
                // Print analyses
                cout << endl << endl;
                //ba.test();
                for (unsigned int i=0; i<specie_option.size(); i++)
                {
                    cout << specie_str[specie_option.at(i)] << endl;
                    oa[i]->test();
                    delete oa[i];
                }
            }
            else
            {
                // Warn user about kind of useless case
                cout << color::set(color::YELLOW) << "Warning: Your current set of command line arguments produces a somewhat useless state. (No outputs are being generated.) Did you mean to include the [-l | --local-range] flag?" << color::clear() << endl;
                
                // No analysis, simply emulate a simulation
                // This particular case is pointless at the moment, but having
                //   analysis be optional is useful under the data_export case.
                csvr_sim csvrs(data_import, specie_option);
                csvrs.run();
            }
        }
        else // If not importing data
        {
            string param_sets;
            int total_width;
            RATETYPE sim_time, anlys_intvl;

            if ( arg_parse::get<int>("c", "cell-total", &cell_total, true) &&
                    arg_parse::get<int>("w", "total-width", &total_width, true) &&
                    arg_parse::get<RATETYPE>("t", "time", &sim_time, true) &&
                    arg_parse::get<RATETYPE>("a", "anlys-intvl", &anlys_intvl, true) &&
                    arg_parse::get<string>("p", "param-sets", &param_sets, true) )
            {
                // Now that we know sim_time
                time_end = arg_parse::get<RATETYPE>("v", "time-end", sim_time);
                cell_end = arg_parse::get<int>("y", "cell-end", cell_total);
                
                // If step_size not set, create stochastic simulation
                RATETYPE step_size =
                    arg_parse::get<RATETYPE>("s", "step-size", 0.0);
                int seed = arg_parse::get<int>("r", "rand-seed", time(0));

                // Warn user that they are not running deterministic sim
                if (step_size == 0.0)
                {
                    cout << color::set(color::YELLOW) << "Running stochastic simulation. To run deterministic simulation, specify a step size using the [-s | --step-size] flag." << color::clear() << endl;
                    cout << "Stochastic simulation seed: " << seed << endl;
                }
                
                simulation_set sim_set = simulation_set(
                        param_sets, arg_parse::get<string>("g", "gradients", ""),
                        arg_parse::get<string>("b", "perturbations", ""), cell_total, 
                        total_width, step_size, anlys_intvl, sim_time, seed);
               

                // Prepare data output
                csvw_sim *csvws[sim_set.getSetCount()];
                string data_export;
                bool do_csvws =
                    arg_parse::get<string>("e", "data-export", &data_export, false);
                if (do_csvws)
                {
                    for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                    {
                        string export_name = data_export;
                        // If multiple sets, set file name to "x_####.y"
                        if (sim_set.getSetCount() > 1)
                        {
                            string data_num = to_string(i);
                            data_num.insert(data_num.begin(),
                                4-data_num.size(), '0');
                            export_name = data_export.substr(0, 
                                data_export.find_last_of(".")) + "_" + data_num +
                                data_export.substr(data_export.find_last_of("."));
                        }
                        csvws[i] = new csvw_sim(export_name, anlys_intvl,
                                time_start, time_end, do_time_col,
                                cell_total, cell_start, cell_end,
                                specie_option, sim_set._sim_set[i]);
                    }
                }
                

                // Analysis optional
                if (do_anlys)
                {
                    // Prepare analyses
                    //BasicAnalysis ba(&sim_set._sim_set[0],cell_start,cell_end,time_start,time_end);
                    OscillationAnalysis *oa
                        [sim_set.getSetCount()][specie_option.size()];
                    for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                    {
                        for (unsigned int j=0; j<specie_option.size(); j++)
                        {
                            oa[i][j] = new OscillationAnalysis(
                                    sim_set._sim_set[i], anlys_intvl, 
                                    local_range, specie_option.at(j),
                                    cell_start,cell_end,time_start,time_end);
                        }
                    }

                    // Run the show
                    sim_set.simulate_sets();

                    // Print analyses
                    cout << endl << endl;
                    //ba.test();
                    for (unsigned int i=0; i<sim_set.getSetCount(); i++)
                    {
                        for (unsigned int j=0; j<specie_option.size(); j++)
                        {
                            cout << "set " << i << " for specie " <<
                                specie_str[specie_option.at(j)] << endl;
                            oa[i][j]->test();
                            delete oa[i][j];
                        }
                    }
                }
                else
                {
                    // Warn user about kind of useless case
                    if (!do_anlys && !do_csvws)
                    {
                        cout << color::set(color::YELLOW) << "Warning: Your current set of command line arguments produces a somewhat useless state. (No outputs are being generated.) Did you mean to use the [-l | --local-range] and/or [-e | --data-export] flag(s)?" << color::clear() << endl;
                    }

                    // No analysis, simply run the simulation and output it to file
                    sim_set.simulate_sets();
                }

                // Memory clean-up for CSV Writer
                for (unsigned int i=0; i<sim_set.getSetCount() && do_csvws; i++)
                {
                    delete csvws[i];
                }
            } // End sim required fields if
        } // End -i or not
    } // End -h|blank or not
}
