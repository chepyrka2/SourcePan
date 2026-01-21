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

Slide::Slide(){}

void Slide::printToConsole(){
  std::cout << bar << std::endl;
  std::cout << title << std::endl;
  if (!image.empty()) std::cout << "*picture (path - " << image << ")*" << std::endl;
  std::cout << body << std::endl;
}

bool Slide::operator==(Slide a){
  return (title==a.title) && (body==a.body) && (image==a.image);
}

bool Slide::operator!=(Slide a){
  return !(*this == a);
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
  std::cout << bar << std::endl << desc << std::endl;
  for(Slide slide : slides) {
    slide.printToConsole();
  }
}

bool Recipe::operator==(Recipe a){
  if((name != a.name) || (desc != a.desc) || (author != a.author) || (date != a.date) || (slides.size() == a.slides.size())) return 0;
  for(int i = 0; i < slides.size(); i++) if(slides[i] != a.slides[i]) return 0;
  return 1;
}

bool Recipe::operator!=(Recipe a){
  return !(*this == a);
}

Recipelist::Recipelist(std::vector<Recipe> recipes){
  this->recipes = recipes;
}

Recipelist::Recipelist(){}

Recipelist Recipelist::searchoutput(std::string search, double precision){
  Recipelist out;
  for(Recipe recipe : recipes){
    if(recipe.lookedFor(search, precision)) out.recipes.push_back(recipe);
  }
  return out;
}

Recipelist Recipelist::operator+(Recipelist a){
  Recipelist first = *this;
  for(int i = 0; i < a.recipes.size(); i++){
    if(std::find(recipes.begin(), recipes.end(), a.recipes[i]) == recipes.end())
      first.recipes.push_back(a.recipes[i]);
  }
  return first;
}

Recipelist Recipelist::operator+(Recipe a){
  if(std::find(this->recipes.begin(),this->recipes.end(), a) != this->recipes.end()) return *this;
  Recipelist sixseven = *this;
  sixseven.recipes.push_back(a);
  return sixseven;
}

Recipelist& Recipelist::operator+=(Recipelist& a){
  *this = *this + a;
  return *this;
}
Recipelist& Recipelist::operator+=(Recipe& a){
  *this = *this + a;
  return *this;
}

Recipe placeholdersalad = Recipe("Olivier Salad with a pitch of failure", "This action failed, but we still decided to give you an Olivier Salad recipe.", "05/01/26", "allrecipes.com", {Slide("Ingridients", "6 potatoes, peeled \n1 carrot, or more to taste \n4 whole eggs \n6 large pickles, cut into cubes \n1 (15 ounce) can peas, drained \n1/2 cup cubed fully cooked ham, or to taste \n1 tablespoon chopped fresh dill, or to taste (optional) \n1/2 cup mayonnaise, or to taste"), Slide("1", "Gather all ingredients. Bring a large pot of water to a boil. "), Slide("2", " Add potatoes, bring to a boil, and cook for 5 to 10 minutes. Add carrots and whole eggs and continue boiling until potatoes are tender, 10 to 15 minutes. "), Slide("3", "Drain and slightly cool mixture."), Slide("4", "Chop potatoes and carrot. Peel and chop eggs."), Slide("5", " Mix potatoes, carrot, eggs, pickles, peas, ham, and dill together in a large bowl. "), Slide("6", "Stir in mayonnaise until salad is evenly coated.")});


// int main(){
//   Recipe water = Recipe("Water", "Just water", "67/67", "67 kid", {Slide("Pour water", "Pour water"), Slide("Tadaam!!", "Yayy you did it")});
//   water.printToConsole();
//   std::cout << water.lookedFor("WATER!");
// }