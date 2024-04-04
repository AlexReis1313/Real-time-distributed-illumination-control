#include "includes/distrControl.hpp"
#include "includes/CanManager.hpp"
#include "includes/vars.h"
#include "includes/my_aux.h"
#include <cmath>
#include <vector>


distrControl *distrControl::instance = nullptr; // Initialize static instance pointer
//the gains corresponding to node 0 are in entry 0, node 1 - entry 1, node 2 - entry 2
std::vector<float> distrControl::gainsVector;

std::vector<std::vector<float>> distrControl::all_d; // Initialized to 0
std::vector<float> distrControl::d_average;
std::vector<float> distrControl::current_lagrange_multipliers;
std::vector<float> distrControl::calculated_d_vector;



bool distrControl::endGAINS_bool = false;
float distrControl::tolerance=0.001;

void distrControl::setUpGains(){
    gainsVector = std::vector<float>(my()->nr_ckechIn_Nodes);
    d_average = std::vector<float>(my()->nr_ckechIn_Nodes);
    current_lagrange_multipliers = std::vector<float>(my()->nr_ckechIn_Nodes);
    calculated_d_vector = std::vector<float>(my()->nr_ckechIn_Nodes);
    all_d = std::vector<std::vector<float>>(my()->nr_ckechIn_Nodes, std::vector<float>(my()->nr_ckechIn_Nodes));
    // Initialize inner vectors separately
    for (int i = 0; i < my()->nr_ckechIn_Nodes; ++i) {
        all_d[i] = std::vector<float>(my()->nr_ckechIn_Nodes);
    }

    //gainsVector.resize(my()->nr_ckechIn_Nodes + 1);
    analogWrite(my()->LED_PIN, 0); //turn off LED
    delay(500);
    if(my()->THIS_NODE_NR == 0){
        Serial.println("I am node 0");
        //measure lux o - external - no light
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MEASURE_NO_LIGHTS, nullptr, 0);
        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        my()->o_lux = Volt2LUX(vss); //Get LDR value in lux
    
        analogWrite(my()->LED_PIN, 4000); //Apply control signal to LED
        unsigned char data[sizeof(int)];
        memcpy(data, &my()->THIS_NODE_NR, sizeof(int));
        Serial.println("Starting to MeasureLights ");
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MEASURE_LIGHTS, data ,sizeof(data) );
        vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        float x_lux = Volt2LUX(vss); //Get LDR value in lux
        //            0 = my()->THIS_NODE_NR;    
        float gain=(x_lux - my()->o_lux) / 4000;
        gainsVector[0] =  gain;
        

        analogWrite(my()->LED_PIN, 0);
        int next_node_nr=my()->THIS_NODE_NR + 1;
        memcpy(data, &next_node_nr, sizeof(int));
        //this informs pico 1 that he should light up. From now on, pico 1 will be in charge
        Serial.print("Passing master token to next node. To node: "); Serial.println(next_node_nr);
        CanManager::loopUntilACK(1 , CanManager::PICO_ID, my_type::NOTIFY_FUTURE_LIGHT, data ,sizeof(data) );
        
    } 
    else{
    Serial.print("I am node ");Serial.println(my()->THIS_NODE_NR);
    }
    //loop non blocking - completing actions - until endGains_bool
    bool executeAction=true;
    while(!endGAINS_bool){ 
        CanManager::canBUS_to_actions_rotine(executeAction);

    }        
    
    Serial.println("End of setUP gains");

}


/*
void initialize_node(){


}


 
void distrControl::computeGlobalMinimum() {
//formulas taken from lecture notes section 5.1 
  for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
    if(i == my()->THIS_NODE_NR ) {
      calculated_d_vector[i] = d_average[i] - ( (1/this->optimization_rho) * ( current_lagrange_multipliers[i] + this->cost ) );
    } else {
      calculated_d_vector[i] = d_average[i] - ( (1/this->optimization_rho) * current_lagrange_multipliers[i] );
    }
  }
}


void distrControl::computeBoundarySolutions() {

  std::vector<float> distrControl::vector_ILB(my()->nr_ckechIn_Nodes, 0f);
  std::vector<float> distrControl::vector_DLB(my()->nr_ckechIn_Nodes, 0f);
  std::vector<float> distrControl::vector_DUB(my()->nr_ckechIn_Nodes, 0f);
  std::vector<float> distrControl::vector_ILBintersectDLB(my()->nr_ckechIn_Nodes, 0f);
  std::vector<float> distrControl::vector_ILBintersectDUB(my()->nr_ckechIn_Nodes, 0f);
 
  std::vector<float> distrControl::Y(my()->nr_ckechIn_Nodes, 0f);

  float Kgains_Y = 0;
  
  //from page 14 of lecture notes on consensus algorithm
  for(int i=0; i<my()->nr_ckechIn_Nodes; i++ ) {
    if(i == my()->THIS_NODE_NR) {
      Y[i] = this->optimization_rho * d_average[my()->THIS_NODE_NR] - this->cost - lagrange_multipliers[my()->THIS_NODE_NR];
    } else {
      Y[i] = this->optimization_rho * d_average[i] - lagrange_multipliers[i];
    }

    Kgains_Y += Y[i]*distrControl::gainsVector[i];
  }


    //solution for S1: from page 20 of lecture notes - ILB
    float aux;
    float squared_gains_norm=pow(std::sqrt( std::inner_product(distrControl::gainsVector.begin(), distrControl::gainsVector.end(), distrControl::gainsVector.begin(), 0.0f)), 2);
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        aux= my()->o_lux - this->current_lower_bound + (1/this->optimization_rho )*Kgains_Y;
        
        vector_ILB[i] = (1/this->optimization_rho )*Y[i] - ((distrControl::gainsVector[i] * aux) / squared_gains_norm);
        
    }
  
    //solution for S2: from page 20 of lecture notes - DLB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_DLB[i] = 0;
        }
        else{
            vector_DLB[i] = (1/this->optimization_rho ) * Y[i];
        }
    }
    //solution for S3: from page 21 of lecture notes - DUB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_DUB[i] = 100;
        }
        else{
            vector_DUB[i] = (1/this->optimization_rho ) * Y[i];
        }
    }
    std::vector<float> auxiliar_vector(my()->nr_ckechIn_Nodes, 0f);

    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        auxiliar_vector[i] = ( distrControl::gainsVector[i] / ( squared_gains_norm - pow(distrControl::gainsVector[my()->THIS_NODE_NR], 2) ));
    }

    //solution for S4: from page 22 of lecture notes - ILB & DLB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_ILBintersectDLB[i] = 0;
        }
        else{
            vector_ILBintersectDLB[i] = 
        }
    }
    //solution for S5: from page 22 of lecture notes - ILB & DUB    
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {   
        if(i == my()->THIS_NODE_NR) {
            vector_ILBintersectDUB[i] = 100;
        }
        else{
            aux =  my()->o_lux - this->current_lower_bound + 100*distrControl::gainsVector[my()->THIS_NODE_NR] - (1/this->optimization_rho )*(distrControl::gainsVector[my()->THIS_NODE_NR]*Y[my()->THIS_NODE_NR] - Kgains_Y) ;
            vector_ILBintersectDUB[i] = (1/this->optimization_rho ) * Y[i]  - ( auxiliar_vector[i] * aux );
        }



  

    //ILB:
    if(i == my()->THIS_NODE_NR) {
      vector_DLB[i] = 0;
      vector_DUB[i] = 100;
      vector_ILBintersectDLB[i] = 0;
      vector_ILBintersectDUB[i] = 100;
    } else {
      vector_DUB[i] = (1/this->optimization_rho ) * zed[i];
      vector_ILBintersectDLB[i] = (1/this->optimization_rho )*zed[i] - 
        ( norm_in_fraction * ( my()->o_lux - this->current_lower_bound - (1/this->optimization_rho )*( distrControl::gainsVector[my()->THIS_NODE_NR]*Y[my()->THIS_NODE_NR] - Kgains_Y ) ) );

      vector_ILBintersectDUB[i] = (1/this->optimization_rho ) * zed[i] - ( norm_in_fraction * ( my()->o_lux - this->current_lower_bound + 100*distrControl::gainsVector[my()->THIS_NODE_NR] + (1/this->optimization_rho )*(distrControl::gainsVector[my()->THIS_NODE_NR]*Y[my()->THIS_NODE_NR] - Kgains_Y) ) );
    }
  }

    
  float bestCost = 10000; 
  std::vector<float> bestVector;  

  if( FeasibilityCheck(vector_ILB) and (computeCost(vector_ILB, my()->THIS_NODE_NR) < bestCost) ) {
    bestCost = computeCost(vector_ILB);
    bestVector = vector_ILB;
  }
  if( FeasibilityCheck(vector_DLB) and (computeCost(vector_DLB, my()->THIS_NODE_NR) < bestCost) ) {
    bestCost = computeCost(vector_DLB);
    bestVector = vector_DLB;
  }
  if( FeasibilityCheck(vector_DUB) and (computeCost(vector_DUB, my()->THIS_NODE_NR) < bestCost) ) {
    bestCost = computeCost(vector_DUB);
    bestVector = vector_DUB;
  }
  if( FeasibilityCheck(vector_ILBintersectDLB) and (computeCost(vector_ILBintersectDLB, my()->THIS_NODE_NR) < bestCost) ) {
    bestCost = computeCost(vector_ILBintersectDLB);
    bestVector = vector_ILBintersectDLB;
  }
  if( FeasibilityCheck(vector_ILBintersectDUB) and (computeCost(vector_ILBintersectDUB, my()->THIS_NODE_NR) < bestCost) ) {
    bestCost = computeCost(vector_ILBintersectDUB);
    bestVector = vector_ILBintersectDUB;
  }
  if(bestCost == 10000) {// if no solution was meet, then assume the last solution
    bestVector = dimmings[my()->THIS_NODE_NR];
  }
  
}



float Consensus::computeCost(const std::vector<float>& d_to_compute) {
  //formula taken from section 3.1 "The ADDM algorithm" of DRTCS lecture notes
  
    float computedCost = 0;
    std::vector<float> d_minus_dAVG(my()->nr_ckechIn_Nodes, 0);

    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        d_minus_dAVG[i] = d_to_compute[i] - distrControl::d_average[i];
        computedCost += distrControl::current_lagrange_multipliers[i] *d_minus_dAVG[i];
    }
    //compute norm
    float norm_d_minus_d = std::sqrt( std::inner_product(d_minus_dAVG.begin(), d_minus_dAVG.end(), d_minus_dAVG.begin(), 0.0f));
    computedCost += 0.5 * (pow(norm_d_minus_d, 2));
    computedCost += this->cost * vector_dimming[my()->THIS_NODE_NR] + this->current_lagrange_multipliers[my()->THIS_NODE_NR] * d_minus_dAVG[my()->THIS_NODE_NR];
    
    return computedCost;
}





void Consensus::calculateAverage() {
  for(int i=0; i< my()->nr_ckechIn_Nodes; i++) {
    d_average[i] = 0;
    for(int j=0; j< my()->nr_ckechIn_Nodes; j++){
      d_average[i] += all_d[j][i];
    }
    d_average[i] = d_average[i] / my()->nr_ckechIn_Nodes;
  }
}

bool Consensus::FeasibilityCheck( const std::vector<float>& d_to_check){    
    float total_lux = 0;

    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(d_to_check[i] < 0 - this->tolerance) {
            return false;
        }
        if(d_to_check[i] > 100 + this->tolerance) {
            return false;
        }
        total_lux +=  distrControl::gainsVector[i] * d_to_check[i];
    }
    
    if( total_lux < (this->current_lower_bound - my()->o_lux - this->tolerance) ) {
        return false;
    }
    return true;
}


 */


void distrControl::set_occupancy(bool new_occupancy) {
  my()->occupancy = new_occupancy;
  set_lower_bound();
}

bool distrControl::get_occupancy() {
    return my()->occupancy;
    }

void distrControl::set_lower_bound_occupied(float new_lower_bound_occupied) {
  lower_bound_occupied = new_lower_bound_occupied;
  set_lower_bound();
}

float distrControl::get_lower_bound_occupied() {
    return lower_bound_occupied;
    }

void distrControl::set_lower_bound_unoccupied(float new_lower_bound_unoccupied) {
  lower_bound_unoccupied = new_lower_bound_unoccupied;
  set_lower_bound();
}

float distrControl::get_lower_bound_unoccupied() {
    return lower_bound_unoccupied;
    }


void distrControl::set_lower_bound() {
  if (my()->occupancy)
    current_lower_bound = lower_bound_occupied;
  else
    current_lower_bound = lower_bound_unoccupied;
}

float distrControl::get_lower_bound() {
    return current_lower_bound;
    }

void distrControl::set_cost(float new_cost) {
    cost = new_cost;
    }

float distrControl::get_cost() {
     return cost;
    }






    