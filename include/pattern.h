#pragma once
#include<bits/stdc++.h>
using namespace std;

const int WORD_LEN= 5;
const int GREEN=2, YELLOW=1, GREY=0;

using pattern= vector<int>;

class patternEngine{
    public:
        pattern getPattern(string& guess, string& ans);

        int encodePattern(pattern& p);

        pattern decodePattern(int x);
};
