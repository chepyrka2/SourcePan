#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include "headers/Levenshtein.hpp"
using namespace std;

string plaintext(string input) {
  string idk;
  for (char c : input){
    if (std::isalpha(c) || std::isdigit(c)){
      idk.push_back(tolower(c));
    }
  }
  return idk;
}

int levenshtein(string input, string output) {
  input = plaintext(input);
  output = plaintext(output);
  unsigned int isize = input.size();
  unsigned int osize = output.size();
  unsigned int **matrix = new unsigned int *[osize + 1];
  for (int i = 0; i <= osize; i++) matrix[i] = new unsigned int[isize + 1];

  matrix[0][0] = 0;
  for (unsigned int i = 1; i <= isize; i++) matrix[0][i] = i;
  for (unsigned int i = 1; i <= osize; i++) matrix[i][0] = i;

  for (unsigned int i = 1; i <= osize; i++) {
    for (unsigned int j = 1; j <=  isize; j++) {
      if (output[i-1] == input[j-1]) matrix[i][j] = matrix[i-1][j-1];
      else matrix[i][j] = 1 + min({matrix[i-1][j-1], matrix[i][j-1], matrix[i-1][j]});
    }
  }
  unsigned int dist = matrix[osize][isize];
  for (unsigned int i = 0; i <= osize; i++) delete[] matrix[i];
  delete[] matrix; 
  return dist;
}

double fuzzy_search_score(string input, string output){
  // return (double)levenshtein(input, output) / (double)max(input.length(), output.length());
  if(input.empty()) return 0.0;
  if(input.length() >= output.length()) return (double)levenshtein(input, output) / min(output.length(), input.length());
  vector<double> scores;
  unsigned int len = input.length();
  unsigned int osize = output.length();
  for(int i = 0; i + len < osize; i++){
    scores.push_back((double)levenshtein(input, output.substr(i, len)) / len);
  }
  vector<double>::iterator min_it = min_element(scores.begin(), scores.end());
  return *min_it;
}

bool is_accurate(double precision, string input, string output) { return fuzzy_search_score(input, output) <= precision; }
