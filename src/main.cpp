#include <raylib.h>
#include <iostream>
#include <string>
#include <cctype>
#include "headers/SRCPANmngr.hpp"
#include <filesystem>
#include "headers/Recipe.hpp"
#include "headers/Levenshtein.hpp"

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
    virtual void draw() = 0;
    virtual void input() = 0;
    virtual void update() = 0;

    virtual ~Scene() = default;
};

class RecipeViewScene : public Scene {
public:
  Recipe recipe = placeholdersalad;

  void draw() override {

  }
};

std::string wrapping(std::string input, int linelength) {
  if (input.empty()) return input;

  std::string wrapped;
  wrapped.reserve(input.size() + input.size() / linelength);
  if (input.size() > 1000) {
    input.erase(input.begin() + 1000);
    input.append("...");
  }

  unsigned int n = 0;

  for (char c: input) {
    wrapped.push_back(c);
    n++;

    if (n == linelength) {
      if (std::isalpha(c)) wrapped.push_back('-');
      if (c != '\n') wrapped.push_back('\n');
      n = 0;
    }
  }

  return wrapped;
}

int main() {
  InitWindow(w, h, title);
  Texture2D test = resize("/home/alex/Downloads/images.jpg", 200, 200);
  SetTargetFPS(fps);
  fs::path font;
  bool haveFont = 1;
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
  if (haveFont) montserrat = LoadFontEx(font.c_str(), 64, NULL, 0);
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTexture(test, 50, 50, WHITE);
    DrawTextEx(montserrat, "676767 Hi 676767", (Vector2){67, 67}, (float)60, 2, RED);
    EndDrawing();
  }
  CloseWindow();
}