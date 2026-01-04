#include <algorithm>
#include <string>
using namespace std;

string plaintext(string input) {
  string idonkno = input;
  for (int i = 0; i < input.size(); i++) {
    if (input[i] == ' ')
      idonkno.erase(i, 1);
  }
  return idonkno;
}

int Levenshtein(string input, string output) {
  input = plaintext(input);
  output = plaintext(output);
  int isize = input.size();
  int osize = output.size();
  int **matrix = new int *[osize + 1];
  for (int i = 0; i <= osize; i++) matrix[i] = new int[isize + 1];

  matrix[0][0] = 0;
  for (int i = 1; i <= isize; i++) matrix[0][i] = i;
  for (int i = 1; i <= osize; i++) matrix[i][0] = i;

  for (int i = 1; i <= osize; i++) {
    for (int j = 1; j <=  isize; j++) {
      if (output[i-1] == input[j-1]) matrix[i][j] = matrix[i-1][j-1];
      if (output[i-1] != input[j-1]) matrix[i][j] = 1 + min({matrix[i-1][j-1], matrix[i][j-1], matrix[i-1][j]});
    }
  }
  int dist = matrix[osize][isize];
  for (int i = 0; i <= osize; i++) delete[] matrix[i];
  delete[] matrix; 
  return dist;
}
