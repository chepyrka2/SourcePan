#include <iostream>
#include <vector>
#include <string>
#include <initializer_list>
#include <algorithm>
#include "Levenshtein.hpp"
#include "Recipe.hpp"

std::string bar(30, '-');

Slide::Slide(std::string title, std::string body, std::string image){
  this->title = title;
  this->body = body;
  this->image = image;
}

Slide::Slide(std::string title, std::string body){
  this->title = title;
  this->body = body;
}

void Slide::printToConsole(){
  std::cout << bar << std::endl;
  std::cout << title << std::endl;
  if (!image.empty()) std::cout << "*picture (path - " << image << ")*" << std::endl;
}

Recipe::Recipe(std::string name, std::string desc, std::string date, std::string author, std::initializer_list<Slide> slides){ // With initializer list
  this->name = name;
  this->desc = desc;
  this->date = date;
  this->author = author;
  this->slides = slides;
}

Recipe::Recipe(std::string name, std::string desc, std::string date, std::string author){ // No initializer list
  this->name = name;
  this->desc = desc;
  this->date = date;
  this->author = author;
}

void Recipe::addSlide(std::string title, std::string body, std::string image) {
  slides.push_back(Slide(title, body, image));
}

void Recipe::removeSlide(int index){
  slides.erase(slides.begin() + index);
}

void Recipe::removeSlide(std::string titleToRemove){
  auto newEnd = std::remove_if(slides.begin(), slides.end(), [&titleToRemove](Slide slide) {
    return slide.title == titleToRemove;
  });
  slides.erase(newEnd, slides.end());
}
bool Recipe::lookedFor(std::string search, double precision) { 
  return
    is_accurate(precision, search, name) ||
    is_accurate(precision, desc, search) ||
    is_accurate(0, date, search) ||
    is_accurate(precision, author, search);
}
void Recipe::printToConsole(){
  std::cout << bar << std::endl;
  std::cout << name << std::endl << author << ' ' << date << std::endl;
  for(Slide slide : slides) {
    slide.printToConsole();
  }
}

// int main(){
//   Recipe water = Recipe("Water", "Just water", "67/67", "67 kid", {Slide("Pour water", "Pour water"), Slide("Tadaam!!", "Yayy you did it")});
//   water.printToConsole();
//   std::cout << water.lookedFor("WATER!");
// }