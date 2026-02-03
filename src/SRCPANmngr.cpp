#include <filesystem>
#include <iostream>
#include <miniz/miniz.h>
#include <cstdlib>
#include <miniz/miniz_zip.h>
#include <fstream>
#include <sstream>
#include <string>
#include "headers/SRCPANmngr.hpp"
#include "headers/Levenshtein.hpp"
#include "../inih/INIReader.h"
#include "headers/Recipe.hpp"

namespace fs = std::filesystem;

void archiveZip(fs::path pth, fs::path out){
  mz_zip_archive zip{};
  if (!mz_zip_writer_init_file(&zip, out.c_str(), 0)) {
    std::cerr << "Failed to init writer!";
    return;
  }
  for(auto& entry : fs::recursive_directory_iterator(pth)){
    std::string relative = fs::relative(entry.path(), pth).generic_string();
    if(fs::is_directory(entry.path())){
      mz_zip_writer_add_mem(&zip, (relative + '/').c_str(), "", 0, MZ_NO_COMPRESSION);
    }
    else mz_zip_writer_add_file(&zip, relative.c_str(), entry.path().c_str(), nullptr, 0, MZ_BEST_COMPRESSION);
  }

  mz_zip_writer_finalize_archive(&zip);
  mz_zip_writer_end(&zip);
}

void extractZip(fs::path pth, fs::path out){
  mz_zip_archive zip{};
  if(!mz_zip_reader_init_file(&zip, pth.c_str(), 0)){
    std::cerr << "Failed!" << mz_zip_get_error_string(zip.m_last_error) << std::endl;
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

void load(Recipelist& toAdd){

  Recipelist loading;
  fs::path recs = homedir() / "recs";
  fs::path pwd;
  std::string name;
  std::string desc;
  std::string author;
  std::string date;
  for(auto& entry : fs::directory_iterator(recs)){
    if(!fs::is_directory(entry.path())) continue;
    pwd = entry.path();
    INIReader ini(pwd / "info.ini");
    name = ini.Get("Info", "Name", "Unknown");
    desc = ini.Get("Info", "Description", "Nothing!");
    author = ini.Get("Info", "Author", "Anonimous J. D.");
    date = ini.Get("Info", "Date", "19.01.26");
    Recipe recipe(name, desc, date, author);
    for(int i = 1;; i++){
      fs::path slidetxtpth = pwd / "slides" / (std::to_string(i) + ".txt");
      if(!fs::exists(slidetxtpth)) break;
      char flags = 0;
      std::ifstream slidetxt(pwd/"slides"/(std::to_string(i) + ".txt"));
      if(!slidetxt.is_open()) recipe = placeholdersalad;
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
    loading += recipe;
  }
  toAdd = loading;
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
    fs::create_directories(homedir() / fs::path("recs"));
  }
  if(!fs::exists(homedir() / fs::path("recs"))) StartSRCPAN();
}

Recipe unpack(fs::path pth){
  if((!fs::exists(pth)) || ((pth.extension() != ".zip") && (pth.extension() != ".srcpan"))) return placeholdersalad;
  if(!fs::exists(homedir() / "recs")) StartSRCPAN();
  fs::path temp = pth;
  temp.replace_extension(".zip");
  temp = homedir() / "recs" / temp.filename();
  if (fs::exists(temp)) fs::remove_all(temp);
  fs::copy(pth, temp, fs::copy_options::overwrite_existing);
  fs::path out = temp;
  out.replace_extension("");
  extractZip(temp, out);
  fs::remove_all(temp);
  INIReader ini((out / "info.ini").string());
  std::string name = ini.Get("Info", "Name", "Unknown");
  std::string desc = ini.Get("Info", "Description", "Nothing!");
  std::string author = ini.Get("Info", "Author", "Anonimous J. D.");
  std::string date = ini.Get("Info", "Date", "19.01.26");
  Recipe recipe(name, desc, date, author);
  for(int i = 1;; i++){
    fs::path slidetxtpth = out / "slides" / (std::to_string(i) + ".txt");
    if(!fs::exists(slidetxtpth)) break;
    char flags = 0;
    std::ifstream slidetxt(out/"slides"/(std::to_string(i) + ".txt"));
    if(!slidetxt.is_open()) return placeholdersalad;
    std::string pic;
    std::getline(slidetxt, pic);
    if(pic.find("Pic:") != std::string::npos) {
      // fs::path  = pic.substr(pic.find("Pic:")+5);
      // fs::path realPath = temp / "pics" / larpingPath.filename();
      // if (fs::exists(realPath)) pic = ;
      std::string fn = pic.substr(pic.find("Pic: ") + 5);
      fs::path pthh = fs::absolute(temp / "pics" / fn);
      pic = pthh.string();
    }
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

void pack(Recipe recipe, fs::path path, fs::path picfold){
  fs::path cwd;
  if(fs::exists(homedir() / "recs" / plaintext(recipe.name))){
    int i = 1;
    while(fs::exists(homedir() / "recs" / (plaintext(recipe.name) + "0" + std::to_string(i)))) {
      if (i <= 100) i++;
      else {
        std::cerr << "Delete any folder with recipes name and a numbers" << std::endl;
        return;
      }
    }
    cwd = homedir() / "recs" / (plaintext(recipe.name) + "0" + std::to_string(i));
  } else {
    cwd = homedir() / "recs" / plaintext(recipe.name);
    for(Slide& slide : recipe.slides) slide.image = "";
  }
  fs::create_directories(cwd);
  fs::create_directories(cwd / "slides");
  fs::create_directories(cwd / "pics");
  fs::copy(fs::absolute(picfold), cwd / "pics", fs::copy_options::recursive);
  std::ofstream info(cwd / "info.ini");
  info << "[Info]\n"
          "Name=" << recipe.name << "\n"
          "Description=" << recipe.desc << "\n"
          "Author=" << recipe.author << "\n"
          "Date=" << recipe.date;
  info.close();
  int i = 1;
  for(Slide slide : recipe.slides){
    std::ofstream slidetxt(cwd / "slides" / (std::to_string(i) + ".txt"));
    if(!slide.image.empty()) slidetxt << "Pic: " << fs::path( slide.image ).filename();
    slidetxt << "\n";
    if(slide.title != std::to_string(i)) slidetxt << "Title: " << slide.title;
    slidetxt << "\n";
    slidetxt << slide.body;
    slidetxt.close();
    i++;
  }
  path.replace_extension(".zip");
  if(fs::exists(path)) fs::remove_all(path);
  archiveZip(cwd, path);
  path.replace_extension(".srcpan");
  if(fs::exists(path)) fs::remove_all(path);
  path.replace_extension(".zip");
  fs::path pth = path;
  pth.replace_extension(".srcpan");
  fs::rename(path, pth);
}

// int main(){
//   // Recipelist recipes;
//   // load(recipes);
//   // recipes.recipes[0].printToConsole();
//   // archiveZip("/home/alex/recs/water", "/home/alex/recs/water.zip");
//   // // unpack("/home/alex/recs/water.zip").printToConsole();
//   // pack(placeholdersalad, "/home/alex/screenshots/water.srcpan");
//   unpack("/home/alex/screenshots/water.srcpan").printToConsole();
// }