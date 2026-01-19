#include <filesystem>
#include <iostream>
#include <miniz/miniz.h>
#include <cstdlib>
#include <miniz/miniz_zip.h>
#include <fstream>
#include <sstream>
#include <string>
#include "SRCPANmngr.hpp"
#include "inih/INIReader.h"
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
  if(!fs::exists(homedir()/fs::path("recs"))) StartSRCPAN();
  fs::path out = homedir() / "recs" / pth.filename();
  fs::path out2 = out;
  fs::copy_file(pth, out, fs::copy_options::overwrite_existing);
  fs::rename(out, out2);
  extractZip(out, out.replace_extension());
  out = out.replace_extension(".zip");
  fs::remove(out);
  out = out.replace_extension();
  INIReader ini((out / "info.ini").string());
  std::string name = ini.Get("Info", "Name", "Unknown");
  std::string desc = ini.Get("Info", "Description", "Nothing!");
  std::string author = ini.Get("Info", "Author", "Anonimous J. D.");
  std::string date = ini.Get("Info", "Date", "19.01.26");
  Recipe recipe(name, desc, date, author);
  // int i = 0;
  // for(const auto& entry : fs::directory_iterator(out/"slides")){
  //   i++;
    // char flags = 0;
    // std::ifstream slidetxt(out/"slides"/(std::to_string(i) + ".txt"));
    // if(!slidetxt.is_open()) return placeholdersalad;
    // std::string pic;
    // std::getline(slidetxt, pic);
    // if(pic.find("Pic:") != std::string::npos) pic = pic.substr(pic.find("Pic:")+5);
    // else flags |= 1;
    // std::string title;
    // std::getline(slidetxt, title);
    // if(pic.find("Title:") != std::string::npos) title = title.substr(title.find("Title:")+7);
    // else title = std::to_string(i);
    // std::stringstream buf;
    // std::string line;
    // while(std::getline(slidetxt, line)) buf << line << "\n";
    // Slide idk;
    // if(flags) idk = Slide(title, buf.str());
    // else idk = Slide(title, buf.str(), pic);
    // recipe.slides.push_back(idk);
  // }

  for(int i = 1;; i++){
    fs::path slidetxtpth = out / "slides" / (std::to_string(i) + ".txt");
    if(!fs::exists(slidetxtpth)) break;
    char flags = 0;
    std::ifstream slidetxt(out/"slides"/(std::to_string(i) + ".txt"));
    if(!slidetxt.is_open()) return placeholdersalad;
    std::string pic;
    std::getline(slidetxt, pic);
    if(pic.find("Pic:") != std::string::npos) pic = pic.substr(pic.find("Pic:")+5);
    else flags |= 1;
    std::string title;
    std::getline(slidetxt, title);
    if(title.find("Title:") != std::string::npos) title = title.substr(title.find("Title:")+7);
    else title = std::to_string(i);
    std::stringstream buf;
    std::string line;
    while(std::getline(slidetxt, line)) buf << line << "\n";
    Slide idk;
    if(flags) idk = Slide(title, buf.str());
    else idk = Slide(title, buf.str(), pic);
    recipe.slides.push_back(idk);
  }
  return recipe;
}

int main(){
  unpack("/home/alex/coding/project/water.srcpan").printToConsole();
}