#include <filesystem>
#include <zip.h>
#include <string>
#include "SRCPANmngr.hpp"
#include "Recipe.hpp"

namespace fs = std::filesystem;

void StartSRCPAN(){
  if(!fs::exists(fs::path("temprecs"))){
    fs::create_directory(fs::path("temprecs"));
  }
}

void QuitSRCPAN(){
  if(fs::exists(fs::path("temprecs"))){
    fs::remove_all(fs::path("temprecs"));
  }
}

Recipe unpack(fs::path pth){
  if((!fs::exists(pth)) || ((pth.extension() != ".srcpan") && (pth.extension() == ".zip"))) return placeholdersalad;
  

}