#include <raylib.h>
#include <iostream>
#include <string>
#include <cctype>
#include "headers/SRCPANmngr.hpp"
#include <filesystem>
#include "headers/Recipe.hpp"
// #include "headers/Levenshtein.hpp"
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
    return Texture2D{};
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

    virtual ~Scene() = default;

    virtual void reload(Recipe recipe) = 0;
};

class SceneManager {
private:
  Scene* current = nullptr;
public:
  std::vector<std::unique_ptr<Scene>> scenes;

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
    // del(1);
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
  for (int i = 0; i < input.size(); i++) {
    if ( (input[i] == ' ') || (input[i] == '\n') ) {
      input.erase(input.begin() + i);
      i--;
    } else {
      break;
    }
  }
  std::string wrapped;
  if (input.size() > limit) {
    input.erase(input.begin() + limit + 1);
    input.append("...");
  }

  unsigned int n = 0;

  for (int i = 0; i < input.size() - 1; i++) {
    wrapped.push_back(input[i]);
    n++;

    if (input[i] == '\n') n = 0;

    if ((i != input.size()) && (n == linelength)) {
      if ((std::isalpha(input[i])) && (std::isalpha(input[i+1]))) wrapped.push_back('-');
      if (input[i] != '\n') wrapped.push_back('\n');
      n = 0;
    }
  }

  wrapped.push_back(input[input.size() - 1]);

  return wrapped;
}

class RecipeViewScene : public Scene {
private:
  Font font;
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

  void input() override {
    if (IsKeyPressed(KEY_ENTER)) {
      sm->scenes[2].get()->reload(recipe);
      sm->setCurrent(2);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      sm->setCurrent((unsigned int) 1);
    }
  }

  void reload(Recipe rec) override {
    this->recipe = rec;
  }
};

class SlideScene : public Scene {
private:
  Recipe recipe = Recipe("", "", "", "");
  unsigned int ind = 0;
  Texture2D texture{};
  SceneManager* sm;
public:
  Font font;
  SlideScene(const Recipe recipe, const Font font, const unsigned int i, SceneManager& sm) {
    this->recipe = recipe;
    this->font = font;
    this->sm = &sm;
    ind = i;
    if (!this->recipe.slides[0].image.empty()) texture = resize(recipe.slides[0].image, 480, 270);
  }

  void draw() override {
    ClearBackground(WHITE);
    if (!this->recipe.slides[ind].image.empty()) DrawTexture(texture, ( w-480 ) / 2, 65, WHITE);
    DrawTextEx(font, wrapping(recipe.slides[ind].title, 30, 29).c_str(), (Vector2) {(w - MeasureTextEx(font, wrapping(recipe.slides[ind].title, 30, 29).c_str(), 40, 2).x) / 2, 20}, 40, 2, BLACK);
    DrawTextEx(font, wrapping(recipe.slides[ind].body, 80, 1000).c_str(), (Vector2){(w - MeasureTextEx(font, wrapping(recipe.slides[ind].body, 80, 1000).c_str(), 30, 2).x) / 2, 350}, 30, 2, BLACK);
  }

  void input() override {
    if (IsKeyPressed(KEY_RIGHT)) {
      if (ind != recipe.slides.size() - 1) ind++;
    }
    if (IsKeyPressed(KEY_LEFT)) {
      if (ind != 0) ind--;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
      sm->setCurrent(1);
    }
  }

  ~SlideScene() override {
    if (texture.id != 0) UnloadTexture(texture);
  }

  void reload(Recipe rec) override {
    recipe = rec;
    if (!this->recipe.slides[ind].image.empty()) texture = resize(recipe.slides[0].image, 480, 270);
    ind = 0;
  }
};

class MenuScene : public Scene {
private:
  Recipelist* rl;
  Recipelist rlm;
  SceneManager* sm;
  Font font;
  bool isWriting = 0;
  std::string search{};
  fs::path appdata;
  Texture2D rld{};
  std::vector<unsigned int> buttons;
  int scroll = 0;
  Texture2D sxsvn{};
public:
  MenuScene(Recipelist& recipelist, SceneManager& scm, const Font& fn) {
    rl = &recipelist;
    rlm = *rl;
    sm = &scm;
    font = fn;
    if (getOS() == 'w') appdata = homedir() / "AppData" / "SorcePan";
    else appdata = homedir() / ".local" / "share" / "SourcePan";
    rld = resize( appdata / "rld.png", 50, 50);
    sxsvn = resize(appdata / "67.jpg", 600, 300);
  }

  void input() override {
    char key = GetCharPressed();
    if (GetMouseWheelMove()) {
      if ( (GetMouseWheelMove() > 0) && (scroll < 320)) {
        scroll += 20;
      }
      if (GetMouseWheelMove() < 0) {
        scroll -= 30;
      }
    }
    if (isWriting) {
      scroll = 0;
      if (IsKeyPressed(KEY_BACKSPACE) && !search.empty()) search.erase(search.end() - 1);
      if (std::isalpha(key) || std::isdigit(key) || ( key == ' ' ) || (key == '.')) search += key;
      if (IsKeyPressed(KEY_ENTER)) {
        rlm = rl->searchoutput(search);
        isWriting = 0;
        std::cout << search << std::endl;
      }
    }
    if (IsKeyPressed(KEY_ESCAPE)) isWriting = 0;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mp = GetMousePosition();

      if (((mp.x >= 300) && (mp.x < 930)) && ((mp.y >= 20 + scroll) && (mp.y <= 70 + scroll))) isWriting = 1;
      if (((mp.x >= 230) && (mp.x <= 280)) && ((mp.y >= 20 + scroll) && (mp.y <= 70 + scroll))) {
        load(*rl);
        rlm = *rl;
        std::cout << "Reloaded!" << std::endl;
      }
      if (((mp.x >= 930) && (mp.x <= 980)) && ((mp.y >= 20 + scroll) && (mp.y <= 70 + scroll))) {
        rlm = rl->searchoutput(search);
      }
    }
  }

  void draw() override {
    std::string searchout = search;
    buttons.clear();
    if (MeasureTextEx(font, searchout.c_str(), 40, 2).x > 610) {
      searchout.erase(searchout.begin(), searchout.end() - 15);
      searchout = "..." + searchout;
    }
    ClearBackground(WHITE);
    DrawTexture(sxsvn, 340, scroll - 300, WHITE);
    DrawRectangleLinesEx(Rectangle(300, 20+scroll, 630, 50), 3, BLACK);
    DrawRectangleRec(Rectangle(930, 20+scroll, 50, 50), BLUE);
    DrawText("Q", 940, 20+scroll, 50, WHITE);
    std::string i = isWriting? "1":"0";
    DrawText(i.c_str(), 0, 0, 20, BLACK);
    DrawTexture(rld, 230, 20+scroll, WHITE);
    DrawTextEx(font, searchout.c_str(), (Vector2) { 310, (float) 25+scroll}, 40, 2, BLACK);
    for (int j = 0; j < rlm.recipes.size(); j++) {
      DrawRectangleLinesEx(Rectangle(0, 100 + 150 * j+scroll, w, 150), 3, BLACK);
      DrawTextEx(font, wrapping(rlm.recipes[j].name, 22, 19).c_str(), (Vector2) {10, (float)100+150*j+5+scroll}, 70, 2, BLACK);
      DrawTextEx(font, wrapping(rlm.recipes[j].desc, 40, 76).c_str(), (Vector2){10, (float) 100+150*j+80+scroll}, 20, 2, BLACK);
      DrawTextEx(font, wrapping(rlm.recipes[j].author, 22, 19).c_str(), (Vector2) {w - MeasureTextEx(font, wrapping(rlm.recipes[j].author, 22, 19).c_str(), 30, 2).x - 10, (float) 100+150*j+10+scroll}, 30, 2, BLACK);
      DrawTextEx(font, rlm.recipes[j].date.c_str(), (Vector2) {w - 10 - MeasureTextEx(font, rlm.recipes[j].date.c_str(), 30, 2).x, (float) 100+150*j+50+scroll}, 30, 2, BLACK);
      buttons.push_back(100+150*j+scroll);
    }
  }
  void reload(Recipe recipe) override{}
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
  Recipe rec = unpack("/home/alex/recs/idk.srcpan");
  // std::unique_ptr<Scene> a = std::make_unique<SlideScene>(rec, montserrat, 0, sm);
  // sm.add(std::move(a));
  // sm.setCurrent((unsigned int) 0);
  Recipelist rl;
  load(rl);
  MenuScene ms = MenuScene(rl, sm, montserrat);
  SetExitKey(KEY_NULL);
  while (!WindowShouldClose()) {
    BeginDrawing();
    try {
      ms.input();
      ms.draw();
    }
    catch (const std::string& e) {
      std::cout << e << '\n';
    }
    EndDrawing();
  }
  CloseWindow();
}