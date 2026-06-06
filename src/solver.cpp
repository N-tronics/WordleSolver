#include "pattern.h"
#include "solver.h"
#include<bits/stdc++.h>
using namespace std;

solver::solver(string fname){
    ifstream file(fname);
    string word;
    while(getline(file,word)){
        all.push_back(word);
    }
    for(int i=0; i<2309; i++) remaining.push_back(i);
    buildMatrix();
}

void solver::buildMatrix(){
    matrix.resize(2309);
    for(int i=0; i<2309; i++) matrix[i].resize(2309);
    patternEngine p;
    for(int i=0; i<2309; i++){
        for(int j=0; j<2309; j++){
            pattern pat= p.getPattern(all[i],all[j]);
            matrix[i][j] = p.encodePattern(pat);
        }
    }
}

void solver::filterWords(pattern& p){
    patternEngine pat;
    int target= pat.encodePattern(p);
    vector<int> new_rem;
    for(auto i: remaining) if(matrix[i][prev_guess]==target) new_rem.push_back(i);
    remaining= new_rem;
}

double solver::entropyScore(int guess){
    unordered_map<int,int> buckets;
    for(auto i: remaining) buckets[matrix[i][guess]]++;
    double total= remaining.size();
    double entropy=0;
    for(auto i:buckets){
        double proportion= i.second/total;
        entropy+= proportion*log2(proportion);
    }
    return -entropy;
}

string solver::bestGuess(){
    double ma= -1e8;
    int maxidx=-1;
    for(auto i: remaining){
        int x=entropyScore(i);
        if(x>ma){
            ma= x;
            maxidx= i; 
        } 
    }
    prev_guess= maxidx;
    return all[maxidx];
}

void solver::reset(){
    vector<int> new_rem;
    for(int i=0; i<2309; i++) new_rem.push_back(i);
    remaining= new_rem;
    prev_guess=-1;
}