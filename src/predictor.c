//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <math.h>



//
// TODO:Student Information
//
const char *studentName = "Dongyu Chen";
const char *studentID   = "A59019803";
const char *email       = "doc004@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint8_t* bht_g;
int ghistory;

uint32_t* pht_l;
uint8_t* bht_l;
uint8_t* chooser;


uint8_t outcome_l;
uint8_t outcome_g;

int** perceptrons;
int phl = 127;
int n_perceptrons = 300;
int threshold;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
switch (bpType) {
    case STATIC:
        break;
    case GSHARE:
        init_predictor_G();
        break;
    case TOURNAMENT:
        init_predictor_T();
        break;
    case CUSTOM:
        init_predictor_C();
        break;
    default:
        printf("Type Undefined.\n");
        break;
    }
    
}


void init_predictor_G() {
    ghistory = 0;
    bht_g = (uint8_t*)malloc((1 << ghistoryBits) * sizeof(uint8_t));
    int bht_entries = 1 << ghistoryBits;
    for (int i = 0; i < bht_entries; i++) {
        bht_g[i] = WN;
    }
}



void init_predictor_T() {
    ghistory = 0;
    pht_l = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    bht_l = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    chooser = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    bht_g = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    for (int i = 0; i < (1 << pcIndexBits); i++) {
        pht_l[i] = 0;
    }
    for (int i = 0; i < (1 << pcIndexBits); i++) {
        bht_l[i] = WN;
    }
    for (int i = 0; i < (1 << ghistoryBits); i++) {
        chooser[i] = WN;
        bht_g[i] = WN;
    }
}


void init_predictor_C() {
    ghistory = 0;
    threshold = (1.93 * phl) + 14;
    perceptrons = (int**)malloc(n_perceptrons * sizeof(int*));
    for (int i = 0; i < n_perceptrons; i++) {
        perceptrons[i] = (int*)malloc((phl + 1) * sizeof(int));
        for (int j = 0; j < phl + 1; j++) {
            perceptrons[i][j] = 1;
        }
    }

}


// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
switch (bpType) {
    case STATIC:
        return TAKEN;
        break;
    case GSHARE:
        return make_prediction_G(pc);
        break;
    case TOURNAMENT:
        return make_prediction_T(pc);
        break;
    case CUSTOM:
        return make_prediction_C(pc);
        break;
    default:
        printf("Type undefined.\n");
        break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

uint8_t make_prediction_G(uint32_t pc) {
    uint32_t tempIndex;
    tempIndex = (ghistory ^ pc) & ((1 << ghistoryBits) - 1);
    switch (bht_g[tempIndex]) {
    case SN:
        return NOTTAKEN;
    case WN:
        return NOTTAKEN;
    case ST:
        return TAKEN;
    case WT:
        return TAKEN;
    default:
        printf("State Undefined.\n");
        return NOTTAKEN;
    }
}


uint8_t make_prediction_T(uint32_t pc) {
    uint32_t chooser_index = ghistory & ((1 << ghistoryBits) - 1);
    uint32_t predictor = chooser[chooser_index];
    
    uint32_t pattern = pc & ((1 << pcIndexBits) - 1);
    uint32_t bht_l_index = pht_l[pattern];
    uint8_t prediction = bht_l[bht_l_index];

    outcome_l = (prediction == WT || prediction == ST) ? TAKEN : NOTTAKEN;
    

    uint32_t bht_index = (ghistory) & ((1 << ghistoryBits) - 1);
    prediction = bht_g[bht_index];

    outcome_g = (prediction == WT || prediction == ST) ? TAKEN : NOTTAKEN;

    return (predictor == WT || predictor == ST) ? outcome_l : outcome_g;
}


uint8_t make_prediction_C(uint32_t pc) {
    int index = (pc >> 2) % n_perceptrons;
    int result = perceptrons[index][0];
    
    uint64_t temp = ghistory;
    for (int i = 1; i < phl + 1; i++) {
        if ((temp & (1 << i)) != 0) {
            result += perceptrons[index][i];
        }
        else {
            result -= perceptrons[index][i];
        }
    }
    return (result > 0) ? TAKEN : NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome){
  //
  //TODO: Implement Predictor training
  //
    switch (bpType) {
        case STATIC:
            break;
        case GSHARE:
            train_predictor_G(pc, outcome);
            break;
        case TOURNAMENT:
            train_predictor_T(pc, outcome);
            break;
        case CUSTOM:
            train_predictor_C(pc, outcome);
            break;
        default:
            printf("Undefined State\n");
            break;
        }
}


void train_predictor_G(uint32_t pc, uint8_t outcome) {
   
    uint32_t bht_entries = 0;
    
    for (int i = 0; i < ghistoryBits; i++) {
        bht_entries = (bht_entries * 2) + 1;
    }

    uint32_t tempIndex = (pc & bht_entries) ^ (ghistory & bht_entries);
    switch (bht_g[tempIndex]) {
    case SN:
        bht_g[tempIndex] = (outcome == TAKEN) ? WN : SN;
        break;
    case WN:
        bht_g[tempIndex] = (outcome == TAKEN) ? WT : SN;
        break;
    case WT:
        bht_g[tempIndex] = (outcome == TAKEN) ? ST : WN;
        break;
    case ST:
        bht_g[tempIndex] = (outcome == TAKEN) ? ST : WT;
        break;
    default:
        printf("Undefined State\n");
        break;
    }
    ghistory = ghistory << 1;
    ghistory = (ghistory | outcome);
}

void train_predictor_T(uint32_t pc, uint8_t outcome) {
    if (outcome_l != outcome_g) {

        switch (chooser[ghistory]) {
        case SN:
            chooser[ghistory] = (outcome_l == outcome) ? WN : SN;
            break;
        case WN:
            chooser[ghistory] = (outcome_l == outcome) ? WT : SN;
            break;
        case WT:
            chooser[ghistory] = (outcome_l == outcome) ? ST : WN;
            break;
        case ST:
            chooser[ghistory] = (outcome_l == outcome) ? ST : WT;
            break;
        default:
            printf("Undefined State\n");
            break;
        }

        
    }

    uint32_t pht_l_index = pc & ((1 << pcIndexBits) - 1);
    uint32_t bht_l_index = pht_l[pht_l_index]; 
    //change state
    switch (bht_l[bht_l_index]) {
    case SN:
        bht_l[bht_l_index] = (outcome == TAKEN) ? WN : SN;
        break;
    case WN:
        bht_l[bht_l_index] = (outcome == TAKEN) ? WT : SN;
        break;
    case WT:
        bht_l[bht_l_index] = (outcome == TAKEN) ? ST : WN;
        break;
    case ST:
        bht_l[bht_l_index] = (outcome == TAKEN) ? ST : WT;
        break;
    default:
        printf("Undefined State\n");
        break;
    }

    
    pht_l[pht_l_index] <<= 1;
    pht_l[pht_l_index] &= ((1 << lhistoryBits) - 1);
    pht_l[pht_l_index] |= outcome;

    switch (bht_g[ghistory]) {
    case SN:
        bht_g[ghistory] = (outcome == TAKEN) ? WN : SN;
        break;
    case WN:
        bht_g[ghistory] = (outcome == TAKEN) ? WT : SN;
        break;
    case WT:
        bht_g[ghistory] = (outcome == TAKEN) ? ST : WN;
        break;
    case ST:
        bht_g[ghistory] = (outcome == TAKEN) ? ST : WT;
        break;
    default:
        printf("Undefined State\n");
        break;
    }
    
    ghistory <<= 1;
    ghistory &= ((1 << ghistoryBits) - 1);
    ghistory |= outcome;

}

void 
train_predictor_C(uint32_t pc, uint8_t outcome) {
    int index = (pc >> 2) % n_perceptrons;
    int result = perceptrons[index][0];
    int i = 0;
    uint64_t temp = ghistory;
    for (i = 1; i < phl + 1; i++) {
        if ((temp & (1 << i)) != 0) {
            result += perceptrons[index][i];
        }
        else {
            result -= perceptrons[index][i];
        }
    }

    int sign = (outcome == TAKEN) ? 1 : -1;
    
    
    if ((result > 0 && outcome == NOTTAKEN) || abs(result) < threshold) {
        for (i = 1; i < phl + 1; i++) {
            if ((temp & (1 << i)) != 0) {
                perceptrons[index][i] = perceptrons[index][i] + sign;
            }
            else {
                perceptrons[index][i] = perceptrons[index][i] - sign;
            }
        }
        perceptrons[index][0] = perceptrons[index][0] + sign;
    }
    ghistory = ((ghistory << 1) | outcome);
    ghistory = ghistory & ((1 << phl) - 1);
}
