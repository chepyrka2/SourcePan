#include <filesystem>
#include <iostream>
#include <zip.h>
#include <cstdlib>
#include <string>
#include "SRCPANmngr.hpp"
#include "Recipe.hpp"

namespace fs = std::filesystem;

char getOS(){
  #if defined(_WIN32) || defined(_WIN64)
    return 'w';
  #elif defined(__linux__)
    return 'l';
  #else
    std::exit(67);
  #endif

}

fs::path homedir(){
  if(getOS() == 'w') return fs::path(std::getenv("USERPROFILE"));
  return std::getenv("HOME");
}

void StartSRCPAN(){
  if(!fs::exists(fs::path("recs"))){
    fs::create_directory(fs::path("recs"));
  }
  if(!fs::exists(fs::path("recs"))) StartSRCPAN();
}

Recipe unpack(fs::path pth){
  if((!fs::exists(pth)) || ((pth.extension() != ".srcpan") && (pth.extension() != ".zip"))) return placeholdersalad;
  if(!fs::exists(fs::path("recs"))) StartSRCPAN();
  fs::path out = fs::path("recs/"+pth.filename().string());
  fs::copy_file(pth, out);
  fs::rename(out, out.replace_extension(".zip"));
  
}

