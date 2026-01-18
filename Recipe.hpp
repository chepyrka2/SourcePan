#ifndef RECIPE_HPP
#define RECIPE_HPP

#include <vector>
#include <string>
#include <initializer_list>



struct Slide{
  std::string title;
  std::string body;
  std::string image;
  Slide(std::string title, std::string body, std::string image);
  Slide(std::string title, std::string body);
  void printToConsole();
};

struct Recipe{  
  std::string name;
  std::string desc;
  std::string date;
  std::string author;
  std::vector<Slide> slides;
  Recipe(std::string name, std::string desc, std::string date, std::string author, std::initializer_list<Slide> slides); // With initializer list
  Recipe(std::string name, std::string desc, std::string date, std::string author); // No initializer list
  void addSlide(std::string title, std::string body, std::string image) ;
  void removeSlide(int index);
  void removeSlide(std::string titleToRemove);
  bool lookedFor(std::string search, double precision = 0.3) ; 
  void printToConsole();
};

struct Recipelist{
  std::vector<Recipe> recipes;
  Recipelist(std::vector<Recipe> recipes);
  Recipelist();
  Recipelist searchoutput(std::string search, double precision = 0.3);
};

extern Recipe placeholdersalad;
#endif