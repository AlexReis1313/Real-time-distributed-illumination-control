//#include "vars_can_bus.h"
#include <iostream>
#include <vector>


class distrControl {
    private:
        distrControl() {}
        static float lower_bound_occupied, lower_bound_unoccupied, current_lower_bound, cost, tolerance, optimization_rho;
        static std::vector<float> d_average;
        static std::vector<float> all_d;
        static std::vector<float> current_lagrange_multipliers;
        static std::vector<float> calculated_d_vector;

    public:
        static distrControl* instance;
        
        static bool endGAINS_bool;
        static std::vector<float> gainsVector;
        static void  setUpGains();
/* 
        static bool Consensus::computeCost( const std::vector<float>& d_to_compute)
        static bool Consensus::FeasibilityCheck( const std::vector<float>& d_to_check)
        static void Consensus::calculateAverage();
 */

        static void set_occupancy(bool new_occupancy);  
        static bool get_occupancy();
        static void set_lower_bound_occupied(float new_lower_bound_occupied);
        static float get_lower_bound_occupied();
        static void set_lower_bound_unoccupied(float new_lower_bound_unoccupied);
        static float get_lower_bound_unoccupied();
        static void set_lower_bound();
        static float get_lower_bound();
        static void set_cost(float new_cost);
        static float get_cost(); 

  


};