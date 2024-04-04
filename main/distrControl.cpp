#include "includes/distrControl.hpp"
#include "includes/CanManager.hpp"
#include "includes/vars.h"
#include "includes/my_aux.h"
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <vector>


distrControl *distrControl::instance = nullptr; // Initialize static instance pointer
//the gains corresponding to node 0 are in entry 0, node 1 - entry 1, node 2 - entry 2
std::vector<float> distrControl::gainsVector;

std::vector<std::vector<float>> distrControl::all_d; // Initialized to 0
std::vector<float> distrControl::d_average;
std::vector<float> distrControl::current_lagrange_multipliers;
std::vector<float> distrControl::calculated_d_vector;

float distrControl::lower_bound_occupied = 20;
float distrControl::lower_bound_unoccupied = 10;
bool distrControl::endGAINS_bool = false;
float distrControl::tolerance=0.001;
float distrControl::optimization_rho= 0.07;
float distrControl::cost=1;
float distrControl::current_lower_bound=10;
int distrControl::sending_vector_entry = 0;

void distrControl::setUpGains(){
    gainsVector = std::vector<float>(my()->nr_ckechIn_Nodes);
    d_average = std::vector<float>(my()->nr_ckechIn_Nodes);
    current_lagrange_multipliers = std::vector<float>(my()->nr_ckechIn_Nodes);
    calculated_d_vector = std::vector<float>(my()->nr_ckechIn_Nodes);
    all_d = std::vector<std::vector<float>>(my()->nr_ckechIn_Nodes, std::vector<float>(my()->nr_ckechIn_Nodes));

    my()->list_Nr_detected_consensus=  std::vector<int>(my()->nr_ckechIn_Nodes);
    my()->list_consesus_received_vector =  std::vector<int>(my()->nr_ckechIn_Nodes);
    //my()->list_Nr_detected_consensus.clear(); 
    //my()->list_consesus_received_vector.clear();

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


void distrControl::initializeNewConsensus(){
  Serial.println("Initializing new consensus");
  my()->consensus_ongoing = true;
  sending_vector_entry = 0;
  float checkValue=-101;
  for(int i = 0; i < my()->nr_ckechIn_Nodes ; i++) {
      current_lagrange_multipliers[i] = 0;
      for(int j = 0; j < my()->nr_ckechIn_Nodes ; j++) {
        all_d[i][j]=checkValue;
      }
    }
}

void distrControl::begin_consensus(){ 
    Serial.println("Sending begin msg consensus");
    my()->sendingConsensus_begin = true;
    my()->list_Ack.clear();
    CanManager::enqueue_message(PICO_ID, my_type::BEGINCONSENSUS, nullptr, 0);
}


void distrControl::ComputeConsensus() {
  Serial.println("Computing consensus");

  calculateAverage();
  computeLagrangeMultipliers();

  calculated_d_vector.clear();
  computeGlobalMinInside(); //calculated_d_vector

  if(!FeasibilityCheck(calculated_d_vector)) {
    Serial.println("Searching for solution in boundary");

    calculated_d_vector.clear();
    computeBoundarySolutions(); //calculated_d_vector
  }

  my()->list_Nr_detected_consensus.clear(); //no node has received complete vector from any other node
  my()->list_consesus_received_vector.clear(); //nr of entries that I have about each node
  my()->list_consesus_received_vector[my()->THIS_NODE_NR] = 3; //I know all my entries so far

  float checkValue=-101;
  for(int i = 0; i < my()->nr_ckechIn_Nodes ; i++) { //
      for(int j = 0; j < my()->nr_ckechIn_Nodes ; j++) {
        all_d[i][j]=checkValue;
      }
  }
  
  all_d[my()->THIS_NODE_NR] = calculated_d_vector;
  Serial.print("Computed vector is: ");
    for (size_t i = 0; i < my()->nr_ckechIn_Nodes; ++i) {
            Serial.print(calculated_d_vector[i],4); // Print current element
            Serial.print(", "); // Print comma unless it's the last element
      }
      Serial.println();
  }  

void distrControl::sendConsensus(){
  //Serial.println("Sending my vector consensus");
  //Serial.println("Sending consensus data deativated");
  
  //for (int i = 0; i < my()->nr_ckechIn_Nodes; ++i) {
  if(sending_vector_entry < my()->nr_ckechIn_Nodes){// && (my()->THIS_NODE_NR==1)){
    
    // Enqueue the wake-up message to be sent to all nodes
    unsigned char data[sizeof(float)];

    memcpy(data, &calculated_d_vector[sending_vector_entry], sizeof(calculated_d_vector[sending_vector_entry]));
    
    switch (sending_vector_entry)
    {
    case 0:
      CanManager::enqueue_message(PICO_ID, my_type::RECEIVECONSENSUS0, data, sizeof(data));
      break;
    case 1:
      CanManager::enqueue_message(PICO_ID, my_type::RECEIVECONSENSUS1, data, sizeof(data));
      break;
    case 2:
      CanManager::enqueue_message(PICO_ID, my_type::RECEIVECONSENSUS2, data, sizeof(data));
      break;
    case 3:
      CanManager::enqueue_message(PICO_ID, my_type::RECEIVECONSENSUS3, data, sizeof(data));
      break;
    
    default:
      break;
    }
    //CanManager::enqueue_message(PICO_ID, my_type::RECEIVECONSENSUS, nullptr, 0);
    sending_vector_entry +=1;
  }
  
 
              
}


 
void distrControl::computeGlobalMinInside() {
//formulas taken from lecture notes section 5.1 
  for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
    if(i == my()->THIS_NODE_NR ) {
      calculated_d_vector[i] = d_average[i] - ( (1/optimization_rho) * ( current_lagrange_multipliers[i] + cost ) );
    } else {
      calculated_d_vector[i] = d_average[i] - ( (1/optimization_rho) * current_lagrange_multipliers[i] );
    }
  }
}


void distrControl::computeBoundarySolutions() {

  std::vector<float> vector_ILB(my()->nr_ckechIn_Nodes, 0.0f);
  std::vector<float> vector_DLB(my()->nr_ckechIn_Nodes, 0.0f);
  std::vector<float> vector_DUB(my()->nr_ckechIn_Nodes, 0.0f);
  std::vector<float> vector_ILBintersectDLB(my()->nr_ckechIn_Nodes, 0.0f);
  std::vector<float> vector_ILBintersectDUB(my()->nr_ckechIn_Nodes, 0.0f);
 
  std::vector<float> Y(my()->nr_ckechIn_Nodes, 0.0f);

  float Kgains_Y = 0;
  
  //from page 14 of lecture notes on consensus algorithm
  for(int i=0; i<my()->nr_ckechIn_Nodes; i++ ) {
    if(i == my()->THIS_NODE_NR) {
      Y[i] =  optimization_rho * d_average[my()->THIS_NODE_NR] -  cost - current_lagrange_multipliers[my()->THIS_NODE_NR];
    } else {
      Y[i] =  optimization_rho * d_average[i] - current_lagrange_multipliers[i];
    }

    Kgains_Y += Y[i]*distrControl::gainsVector[i];
  }


    //solution for S1: from page 20 of lecture notes - ILB
    float aux;
    float squared_gains_norm=pow(std::sqrt( std::inner_product(distrControl::gainsVector.begin(), distrControl::gainsVector.end(), distrControl::gainsVector.begin(), 0.0f)), 2);
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        aux= my()->o_lux -  current_lower_bound + (1/ optimization_rho )*Kgains_Y;
        
        vector_ILB[i] = (1/ optimization_rho )*Y[i] - ((distrControl::gainsVector[i] * aux) / squared_gains_norm);
        
    }
  
    //solution for S2: from page 20 of lecture notes - DLB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_DLB[i] = 0;
        }
        else{
            vector_DLB[i] = (1/ optimization_rho ) * Y[i];
        }
    }
    //solution for S3: from page 21 of lecture notes - DUB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_DUB[i] = 100;
        }
        else{
            vector_DUB[i] = (1/ optimization_rho ) * Y[i];
        }
    }
    std::vector<float> auxiliar_vector(my()->nr_ckechIn_Nodes, 0.0f);

    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        auxiliar_vector[i] = ( distrControl::gainsVector[i] / ( squared_gains_norm - pow(distrControl::gainsVector[my()->THIS_NODE_NR], 2) ));
    }

    //solution for S4: from page 22 of lecture notes - ILB & DLB
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(i == my()->THIS_NODE_NR) {
            vector_ILBintersectDLB[i] = 0;
        }
        else{
            aux= ( my()-> o_lux -  current_lower_bound - (1/ optimization_rho )*( distrControl::gainsVector[my()->THIS_NODE_NR]*Y[my()->THIS_NODE_NR] - Kgains_Y ) );
            vector_ILBintersectDLB[i] = (1/ optimization_rho )*Y[i] -  ( auxiliar_vector[i] * aux );
        }
    }
    //solution for S5: from page 22 of lecture notes - ILB & DUB    
    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {   
        if(i == my()->THIS_NODE_NR) {
            vector_ILBintersectDUB[i] = 100;
        }
        else{
            aux =  my()->o_lux -  current_lower_bound + 100*distrControl::gainsVector[my()->THIS_NODE_NR] - (1/ optimization_rho )*(distrControl::gainsVector[my()->THIS_NODE_NR]*Y[my()->THIS_NODE_NR] - Kgains_Y) ;
            vector_ILBintersectDUB[i] = (1/ optimization_rho ) * Y[i]  - ( auxiliar_vector[i] * aux );
        }
  }

  //Now we will check which is the solution that gives the best cost and is feasible
  float bestCost = 10000;

  if( FeasibilityCheck(vector_ILB) && (computeCost(vector_ILB) < bestCost) ) {
    bestCost = computeCost(vector_ILB);
    calculated_d_vector = vector_ILB;
  }
  if( FeasibilityCheck(vector_DLB) && (computeCost(vector_DLB) < bestCost) ) {
    bestCost = computeCost(vector_DLB);
    calculated_d_vector = vector_DLB;
  }
  if( FeasibilityCheck(vector_DUB) && (computeCost(vector_DUB) < bestCost) ) {
    bestCost = computeCost(vector_DUB);
    calculated_d_vector = vector_DUB;
  }
  if( FeasibilityCheck(vector_ILBintersectDLB) && (computeCost(vector_ILBintersectDLB) < bestCost) ) {
    bestCost = computeCost(vector_ILBintersectDLB);
    calculated_d_vector = vector_ILBintersectDLB;
  }
  if( FeasibilityCheck(vector_ILBintersectDUB) && (computeCost(vector_ILBintersectDUB) < bestCost) ) {
    bestCost = computeCost(vector_ILBintersectDUB);
    calculated_d_vector = vector_ILBintersectDUB;
  }
  if(bestCost == 1000000) {// if no solution was meet, then assume the last solution
    for(int i = 0; i < my()->nr_ckechIn_Nodes ; i++) {
      if( all_d[my()->THIS_NODE_NR][i] ==-101){
      calculated_d_vector[i] =0;
      }
      else{
      calculated_d_vector[i] = all_d[my()->THIS_NODE_NR][i];
      }
      
    }
  }
  
  
}

void distrControl::computeLagrangeMultipliers(){
for(int i = 0; i < my()->nr_ckechIn_Nodes ; i++) {
    current_lagrange_multipliers[i] += optimization_rho * ( calculated_d_vector[i] - d_average[i] );
  }
}





void distrControl::calculateAverage() {
  for(int i=0; i< my()->nr_ckechIn_Nodes; i++) {
    d_average[i] = 0;
    for(int j=0; j< my()->nr_ckechIn_Nodes; j++){
      d_average[i] += all_d[j][i];
    }
    d_average[i] = d_average[i] / my()->nr_ckechIn_Nodes;
  }
}

float distrControl::computeCost(const std::vector<float>& d_to_compute) {
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
    computedCost +=  cost * calculated_d_vector[my()->THIS_NODE_NR] +  current_lagrange_multipliers[my()->THIS_NODE_NR] * d_minus_dAVG[my()->THIS_NODE_NR];
    
    return computedCost;
}

bool distrControl::FeasibilityCheck( const std::vector<float>& d_to_check){    
    float total_lux = 0;

    for(int i = 0; i < my()->nr_ckechIn_Nodes; i++) {
        if(d_to_check[i] < 0 -  tolerance) {
            return false;
        }
        if(d_to_check[i] > 100 +  tolerance) {
            return false;
        }
        total_lux +=  distrControl::gainsVector[i] * d_to_check[i];
    }
    
    if( total_lux < ( current_lower_bound - my()->o_lux -  tolerance) ) {
        return false;
    }
    return true;
}


 void distrControl::updateD_values(const std::vector<float>& d_to_update, int node){
    all_d[node]=d_to_update;
 }


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
  float previous_lower_bound =  current_lower_bound;
  if (my()->occupancy)
    current_lower_bound = lower_bound_occupied;
  else
    current_lower_bound = lower_bound_unoccupied;

  if (previous_lower_bound !=  current_lower_bound){
    //begin the consensus
    begin_consensus();
  }
}

float distrControl::get_lower_bound() {
    return current_lower_bound;
    }

void distrControl::set_cost(float new_cost) {
    if (cost != new_cost){
      cost = new_cost;
      begin_consensus();
    }
    
    }

float distrControl::get_cost() {
     return cost;
    }






    