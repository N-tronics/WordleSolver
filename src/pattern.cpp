#include "pattern.h"

pattern patternEngine::getPattern(string& guess, string& ans){
            pattern p(WORD_LEN,0);
            unordered_map<char,int> a;
            //holds remaining usable chars
            for(int i=0; i<WORD_LEN; i++){
                a[ans[i]]++;
                if(guess[i]==ans[i]){ 
                    p[i]=2;
                    a[ans[i]]--;
                }
            }
            for(int i=0; i<WORD_LEN; i++){
                if(p[i]==2) continue;
                if(a.find(guess[i])!=a.end()){
                    if(a[guess[i]]>0){
                        p[i]=1;
                        a[guess[i]]--;
                    }
                    else p[i]=0;
                }
                else p[i]=0;
            }
            return p;
}
int patternEngine::encodePattern(pattern& p){
            int ans=0;
            for(auto i: p) ans= ans*3 + i;
            return ans;
}
pattern patternEngine::decodePattern(int x){
            pattern ans(WORD_LEN);
            for(int i=WORD_LEN-1; i>=0; i--){
                ans[i]= x%3;
                x=x/3;
            }
            return ans;
}