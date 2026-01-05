#ifndef SRCPANMNGR_HPP
#define SRCPANMNGR_HPP

#include <filesystem>
#include "Recipe.hpp"
#include <string>
namespace fs = std::filesystem;

Recipe unpack(fs::path pth);
void pack(Recipe recipe, std::string path);
void StartSRCPAN();
void QuitSRCPAN();

#endif