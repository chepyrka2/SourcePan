//
// Created by alex on 1/31/26.
//
#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "headers/Recipe.hpp"
#include "headers/SRCPANmngr.hpp"

namespace fs = std::filesystem;

int main() {
  std::string rn, ra, rd, rt;
  std::cout << "Name of the recipe ";
  std::cin >> rn;
  std::cout << "Description ";
  std::cin >> rd;
  std::cout << "Author ";
  std::cin >> ra;
  auto rightnow = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(rightnow);
  std::tm tm = *std::localtime(&t);
  std::stringstream ss;
  ss << std::put_time(&tm, "%d.%m.%Y");
  rt = ss.str();
}