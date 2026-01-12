#ifndef SRCPANMNGR_HPP
#define SRCPANMNGR_HPP

#include <filesystem>
#include "Recipe.hpp"
#include <string>
namespace fs = std::filesystem;

void extractZip(fs::path pth, fs::path out);
fs::path homedir();
char getOS();
Recipe unpack(fs::path pth);
void pack(Recipe recipe, std::string path);
void StartSRCPAN();

#endif