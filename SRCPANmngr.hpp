#ifndef SRCPANMNGR_HPP
#define SRCPANMNGR_HPP

#include <filesystem>
#include "Recipe.hpp"

namespace fs = std::filesystem;

void archiveZip(fs::path pth, fs::path out);
void extractZip(fs::path pth, fs::path out);
void load(Recipelist& toAdd);
fs::path homedir();
char getOS();
Recipe unpack(fs::path pth);
void pack(Recipe recipe, fs::path path);
void StartSRCPAN();

#endif