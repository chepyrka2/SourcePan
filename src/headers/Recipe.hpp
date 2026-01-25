#ifndef RECIPE_HPP
#define RECIPE_HPP

#include <vector>
#include <string>
#include <initializer_list>



struct Slide{
  std::string title = "";
  std::string body = "";
  std::string image = "";
  Slide(std::string title, std::string body, std::string image);
  Slide(std::string title, std::string body);
  Slide();
  void printToConsole();
  bool operator==(Slide a);
  bool operator!=(Slide a);
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
  bool operator==(Recipe a);
  bool operator!=(Recipe a);
};


struct Recipelist{
  std::vector<Recipe> recipes;
  Recipelist(std::vector<Recipe> recipes);
  Recipelist();
  Recipelist searchoutput(std::string search, double precision = 0.3);
  Recipelist operator+(Recipelist a);
  Recipelist operator+(Recipe a);
  Recipelist& operator+=(Recipe& a);
  Recipelist& operator+=(Recipelist& a);
};

extern Recipe placeholdersalad;
#endif