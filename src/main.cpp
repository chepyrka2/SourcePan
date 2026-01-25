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

std::string wrapping(std::string input, int linelength) {
  if (input.empty()) return input;

  std::string wrapped;
  wrapped.reserve(input.size() + input.size() / linelength);

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
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    // DrawText(
    //   wrapping("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aliquam laoreet est turpis, finibus tempus velit molestie at. Integer quis accumsan ante, a dictum tellus. ",50).c_str(),
    //   50, 50, 20, BLACK);
    DrawTexture(test, 50, 50, WHITE);
    EndDrawing();
  }
  CloseWindow();
}