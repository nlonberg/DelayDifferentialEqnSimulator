#include "basic.hpp"

using namespace std;

/*
 * UPDATE_AVERAGES
 * arg "start": context iterator to access newest conc level with
 * arg "c": the cell the context inhabits
 * updates average concentration levels across all cells and per cell
*/
void BasicAnalysis :: update_averages(const ContextBase& start, int c){
	
	for (int s=0; s<NUM_SPECIES; s++){
		specie_id sid = (specie_id) s;
		averages[s] += start.getCon(sid);
		avgs_by_context[c][s] += start.getCon(sid);
	}
}

/*
 * UPDATE_MINMAX
 * arg "start": context iterator to access newest conc level with
 * arg "c": the cell the context inhabits
 * updates min and max conc levels across all cells and per cell
*/
void BasicAnalysis :: update_minmax(const ContextBase& start, int c){

	for (int s=0; s<NUM_SPECIES; s++){
		specie_id sid = static_cast<specie_id>(s);
		RATETYPE conc = start.getCon(sid);
		if (conc > maxs[s]){
			maxs[s] = conc;
		}
		if (conc < mins[s]){
			mins[s] = conc;
		}
		if (conc > maxs_by_context[c][s]){
			maxs_by_context[c][s] = conc;
		}
		if (conc < mins_by_context[c][s]){
			mins_by_context[c][s] = conc;
		}
	}
}

void BasicAnalysis :: finalize(){
    
    for (int c=0; c<max-min; c++){
        for (int s=0; s<NUM_SPECIES; s++){
            if (c==0){
                averages[s] = averages[s]/time;
            }
            avgs_by_context[c][s] = avgs_by_context[c][s] / time;
        }
    }
}
