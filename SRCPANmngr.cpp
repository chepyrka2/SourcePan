#include <filesystem>
#include <iostream>
#include <miniz/miniz.h>
#include <cstdlib>
#include <miniz/miniz_zip.h>
#include <ostream>
#include <string>
#include "SRCPANmngr.hpp"
#include "INIReader.h"
#include "Recipe.hpp"

namespace fs = std::filesystem;

void extractZip(fs::path pth, fs::path out){
  mz_zip_archive zip{};
  if(!mz_zip_reader_init_file(&zip, pth.c_str(), 0)){
    std::cerr << "Failed!" << std::endl;
    return;
  }

  fs::create_directories(out);

  int filesCount = mz_zip_reader_get_num_files(&zip);
  for(int i = 0; i < filesCount;  i++){
    mz_zip_archive_file_stat file;
    mz_zip_reader_file_stat(&zip, i, &file);

    fs::path outFile = out / file.m_filename;

    if(mz_zip_reader_is_file_a_directory(&zip, i)){
      fs::create_directories(outFile);
      continue;
    }

    fs::create_directories(outFile.parent_path());

    if(!mz_zip_reader_extract_to_file(&zip, i, outFile.c_str(), 0)){
      std::cerr << "Extract failed! " << file.m_filename << std::endl;
      mz_zip_reader_end(&zip);
      return;
    }
  }

  mz_zip_reader_end(&zip);
  return;
}

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
  if(!fs::exists(homedir() / fs::path("recs"))){
    fs::create_directory(homedir() / fs::path("recs"));
  }
  if(!fs::exists(homedir() / fs::path("recs"))) StartSRCPAN();
}

Recipe unpack(fs::path pth){
  if((!fs::exists(pth)) || ((pth.extension() != ".srcpan") && (pth.extension() != ".zip"))) return placeholdersalad;
  if(!fs::exists(fs::path("recs"))) StartSRCPAN();
  fs::path out = homedir() / "recs" / pth.filename();
  fs::copy_file(pth, out);
  fs::rename(out, out.replace_extension(".zip"));
  
}

