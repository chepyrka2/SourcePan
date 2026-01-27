#include <raylib.h>
#include <iostream>
#include <string>
#include <cctype>
#include "headers/SRCPANmngr.hpp"
#include <filesystem>
#include "headers/Recipe.hpp"
#include "headers/Levenshtein.hpp"
#include <vector>
#include <memory>

namespace fs = std::filesystem;

const int w = 1280;
const int h = 720;
const char title[] = "SourcePan";
const int fps = 60;

Texture2D resize(fs::path img, int x, int y) {
  if (!fs::exists(img)) {
    std::cerr << "Image not found" << std::endl;
    exit(1);
  }
  Image image = LoadImage(img.c_str());
  ImageResize(&image, x, y);
  Texture2D out = LoadTextureFromImage(image);
  SetTextureFilter(out, TEXTURE_FILTER_BILINEAR);
  UnloadImage(image);
  return out;
}

class Scene {
  public:
    Font font;
    virtual void draw() = 0;
    virtual void input() = 0;

    virtual ~Scene() = default;
};

class SceneManager {
private:
  std::vector<std::unique_ptr<Scene>> scenes;
  Scene* current = nullptr;
public:
  void add(std::unique_ptr<Scene> scene) {
    scenes.push_back(std::move(scene));
  }

  void del(unsigned int ind) {
    scenes.erase(scenes.begin() + ind);
  }

  void del(Scene* scene) {
    for (int i = 0; i < scenes.size(); i++) {
      if (scenes[i].get() == scene) del(i);
    }
  }

  void setCurrent(Scene* scene) {
    current = scene;
  }

  void setCurrent(unsigned int ind) {
    current = scenes[ind].get();
  }

  void draw() {
    current->draw();
  }

  void input() {
    current->input();
  }

};


std::string wrapping(std::string input, int linelength, int limit) {
  if (input.empty()) return input;

  std::string wrapped;
  if (input.size() > limit) {
    input.erase(input.begin() + limit + 1);
    input.append("...");
  }

  unsigned int n = 0;

  for (int i = 0; i < input.size(); i++) {
    wrapped.push_back(input[i]);
    n++;

    if ((i != input.size()) && (n == linelength)) {
      if ((std::isalpha(input[i])) && (std::isalpha(input[i+1]))) wrapped.push_back('-');
      if (input[i] != '\n') wrapped.push_back('\n');
      n = 0;
    }
  }

  return wrapped;
}

class RecipeViewScene : public Scene {
private:
  SceneManager* sm;
public:
  Recipe recipe = placeholdersalad;
  RecipeViewScene(Recipe recipe, Font font, SceneManager& sm) {
    this->font = font;
    this->recipe = recipe;
    this->sm = &sm;
  }

  void draw() override {
    ClearBackground(WHITE);
    DrawTextEx(font, wrapping(recipe.name, 30, 59).c_str(), (Vector2){50, 35}, 40, 2, BLACK);
    DrawTextEx(font, wrapping(recipe.date, 10000, 10000).c_str(), (Vector2){w - 10 - MeasureTextEx(font, recipe.date.c_str(), 27, 2).x, 5}, 27, 2, BLACK);
    DrawTextEx(font, wrapping(recipe.author, 31, 30).c_str(), (Vector2){w - 20 - MeasureTextEx(font, recipe.date.c_str(), 27, 2).x - MeasureTextEx(font, recipe.author.c_str(), 27, 2).x, 5}, 27, 2, BLACK);
    DrawTextEx(font, wrapping(recipe.desc, 80, 1600).c_str(), (Vector2){50, 120}, 20, 2, BLACK);
    DrawTextEx(font, "Esc - To menu\nEnter - View", (Vector2){w - 10 - MeasureTextEx(font, "Esc - To menu\nEnter - View", 30, 2).x, h - 10 - MeasureTextEx(font, "Esc - To menu\nEnter - View", 30, 2).y}, 30, 2, BLACK);
  }

  void input() override {}
};

int main() {
  InitWindow(w, h, title);
  // Texture2D test = resize("/home/alex/Downloads/images.jpg", 200, 200);
  SetTargetFPS(fps);
  fs::path font;
  bool haveFont = 1;
  SceneManager sm;
  if (getOS() == 'l') {
    if (!fs::exists(homedir() / ".local" / "share" / "SourcePan")) fs::create_directories (homedir() / ".local" / "share" / "SourcePan");
    font = homedir() / ".local" / "share" / "SourcePan" / "font.ttf";
  } else {
    if (!fs::exists(homedir() / "AppData" / "SourcePan")) fs::create_directories(homedir() / "AppData" / "SorcePan");
    font = homedir() / "AppData" / "SourcePan" / "font.ttf";
    if (!fs::exists(font) && !fs::exists(homedir() / "AppData" / "Local" / "Microsoft" / "Windows" / "Fonts" / "Montserrat-Regular.ttf")) haveFont = 0;
    if (fs::exists(homedir() / "AppData" / "Local" / "Microsoft" / "Windows" / "Fonts" / "Montserrat-Regular.ttf")) {
      fs::copy(homedir() / "AppData" / "Local" / "Microsoft" / "Windows" / "Fonts" / "Montserrat-Regular.ttf", homedir() / "AppData" / "SourcePan" / "font.ttf", fs::copy_options::overwrite_existing);
    }
  }
  Font montserrat;
  if (haveFont) montserrat = LoadFontEx(font.c_str(), 100, NULL, 0);
  std::unique_ptr<Scene> a = std::make_unique<RecipeViewScene>(placeholdersalad, montserrat, sm);
  sm.add(std::move(a));
  sm.setCurrent((unsigned int)0);
  while (!WindowShouldClose()) {
    BeginDrawing();
    sm.draw();
    EndDrawing();
  }
  CloseWindow();
}