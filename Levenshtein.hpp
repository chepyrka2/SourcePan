#ifndef LEVENSHTEIN_HPP
#include <string>
#define LEVENSHTEIN_HPP

std::string plaintext(std::string input);
int levenshtein(std::string input, std::string output);
double fuzzy_search_score(std::string input, std::string output);
bool is_accurate(double precision, std::string input, std::string output);

#endif