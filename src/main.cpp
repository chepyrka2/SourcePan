#include <raylib.h>
#include <iostream>
#include <string>
#include <cctype>
#include "headers/SRCPANmngr.hpp"
#include <filesystem>
#include "headers/Recipe.hpp"
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
      sm->setCurrent((unsigned int) 0);
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
  Texture2D ld{};
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
    ld = resize(appdata / "ld.png", 50, 50);
    sxsvn = resize(appdata / "67.jpg", 600, 300);
  }

  void input() override {
    char key = GetCharPressed();
    if (GetMouseWheelMove()) {
      if ( (GetMouseWheelMove() > 0) && (scroll < 0)) { // < 320 for an easter egg
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
      else isWriting = 0;
      if (((mp.x >= 230) && (mp.x <= 280)) && ((mp.y >= 20 + scroll) && (mp.y <= 70 + scroll))) {
        load(*rl);
        rlm = *rl;
        std::cout << "Reloaded!" << std::endl;
      }
      if (((mp.x >= 930) && (mp.x <= 980)) && ((mp.y >= 20 + scroll) && (mp.y <= 70 + scroll))) {
        rlm = rl->searchoutput(search);
        isWriting = 0;
      }
      if (((mp.x >= 990) && (mp.x <= 1040)) && ((mp.y >= 20+scroll) && (mp.y <= 70+scroll))) {
        sm->setCurrent(3);
      }
      for (int z = 0; z < buttons.size(); z++) {
        if ( mp.y > buttons[z] ) {
          sm->scenes[1].get()->reload(rlm.recipes[z]);
          sm->setCurrent(1);
          break;
        }
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
    DrawTexture(ld, 990, 20+scroll, WHITE);
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

class LoadScene : public Scene {
private:
  SceneManager* sm;
  Font font;
  Recipelist* rl;
  std::string inpt{};
  bool isWriting = 0;
public:
  LoadScene(SceneManager& scm, Font fnt, Recipelist& rll) {
    sm = &scm;
    font = fnt;
    rl = &rll;
  }

  void input() override {
    unsigned char c = GetCharPressed();
    if (isWriting) {
      if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_V)) {
        inpt += GetClipboardText();
      } else {
        if (c) {
          if (std::isalpha(c) || std::isdigit(c) || c == '/' || c == '\\' || c == ':' || c == '.') {
            inpt += c;
            std::cout << inpt << std::endl;
          }
        }
      }
      if (IsKeyPressed(KEY_BACKSPACE) && !inpt.empty()) inpt.erase(inpt.end() - 1);
      if (IsKeyPressed(KEY_ESCAPE)) isWriting = 0;
      if (IsKeyPressed(KEY_ENTER)) {
        try {
        isWriting = 0;
        if (!fs::exists(inpt)) std::cerr << "No such path! \n";
        else if ( (fs::path(inpt).extension() != ".srcpan") && (fs::path(inpt).extension() != ".zip") ) std::cerr << "Wrong extention twin\n";
        else {
          unpack(fs::absolute(inpt));
          rl->recipes.clear();
          load(*rl);
          std::cout << "Added! New amount: " << rl->recipes.size() << std::endl;
        }
        }
        catch (const std::string e) {
          std::cerr << e << '\n';
        }
      }
    } else if (IsKeyPressed(KEY_ESCAPE)) {
      inpt.clear();
      sm->setCurrent((unsigned int) 0);
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mp = GetMousePosition();
      if ((mp.y > (h - 60) / 2) && (mp.y < (h-60)/2+60)) isWriting = 1;
      else isWriting = 0;
    }
  }

  void draw() override {
    std::string toOut = inpt;
    if (toOut.size() > 50) toOut.erase(toOut.begin(), toOut.begin() + (toOut.size() - 47));
    ClearBackground(WHITE);
    DrawRectangleLinesEx(Rectangle(0, ( h-60 ) / 2, w, 60), 3, BLACK);
    if (inpt.empty()) DrawTextEx(font, "Enter the path", (Vector2) { 5, (h - 60) / 2 + 5}, 50, 2, GRAY);
    else DrawTextEx(font, toOut.c_str(), (Vector2) { 5, (h - 60) / 2 + 5}, 50, 2, BLACK);
    std::string i = isWriting? "1":"0";
    DrawText(i.c_str(), 0, 0, 20, BLACK);

  }

  void reload(Recipe recipe) override {}
};

class RecipeMakerScene : public Scene {
private:
  Font font{};
  SceneManager* sm;
  Recipe rec;
  std::string rn, rd, ra, rdate;
  int isWriting = 0;
public:
  RecipeMakerScene(Font fnt, SceneManager& scm) {
    font = fnt;
    sm = &scm;
  }

  void draw() override {
    ClearBackground(WHITE);

    DrawTextEx(font, "Name", (Vector2) {30, 100}, 25, 2, BLACK);
    DrawRectangleLinesEx(Rectangle(0, 130, w, 50), 3, BLACK);
    DrawTextEx(font, rn.c_str(), (Vector2){10, 135}, 40, 2, BLACK);

    DrawTextEx(font, "Description", (Vector2) {30, 200}, 25, 2, BLACK);
    DrawRectangleLinesEx(Rectangle(0, 230, w, 250), 3, BLACK);
    DrawTextEx(font, rd.c_str(), (Vector2){10, 235}, 40, 2, BLACK);

    DrawTextEx(font, "Author", (Vector2) {30, 200}, 25, 2, BLACK);
    DrawRectangleLinesEx(Rectangle(0, 530, w, 50), 3, BLACK);
    DrawTextEx(font, ra.c_str(), (Vector2){10, 335}, 40, 2, BLACK);

    DrawTextEx(font, "Date", (Vector2) {30, 600}, 25, 2, BLACK);
    DrawRectangleLinesEx(Rectangle(0, 630, w, 50), 3, BLACK);
    DrawTextEx(font, rdate.c_str(), (Vector2){10, 435}, 40, 2, BLACK);

    DrawRectangle(w-100, h - 60, 100, 60, BLUE);
    DrawRectangle(w - 90, h - 36, 50, 12, WHITE);
    DrawTriangle((Vector2) {w - 40, h - 48}, (Vector2) {w - 40, h - 12}, (Vector2) {w - 10, h - 30}, WHITE);
  }

  void input() override {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mp = GetMousePosition();
      if ((mp.x < w) && (mp.x > w - 100) && (mp.y < h) && (mp.y > h - 60)) {
        std::cout << "Ts pressed" << std::endl;
        rec = Recipe(rn, rd, rdate, ra);
      }
      else if ((mp.y > 130) && (mp.y < 180)) isWriting = 1;
      else if ((mp.y > 230) && (mp.y < 280)) isWriting = 2;
      else if ((mp.y > 530) && (mp.y < 580)) isWriting = 3;
      else if ((mp.y > 630) && (mp.y < 680)) isWriting = 4;
      else isWriting = 0;
    }

    char c = GetCharPressed();
    switch (isWriting) {
      case 1:
        if ((rn.size() < 59) && (std::isalpha(c) || std::isdigit(c) || c == ' ' || c == '.' || c == ',' || c == ';')) rn += c;
        break;
      case 2:
        if ((rd.size() < 1600) && (std::isalpha(c) || std::isdigit(c) || c == ' ' || c == '.' || c == ',' || c == ';')) rd += c;
        break;
      case 3:
        if ((ra.size() < 30) && (std::isalpha(c) || std::isdigit(c) || c == ' ' || c == '.' || c == ',' || c == ';')) ra += c;
        break;
      case 4:
        if ((rdate.size() < 15) && (std::isalpha(c) || std::isdigit(c) || c == ' ' || c == '.' || c == ',' || c == ';')) rdate += c;
        break;
      default:
        break;
    }

    if (IsKeyPressed(KEY_ESCAPE)) sm->setCurrent((unsigned int) 0);
    if (IsKeyPressed(KEY_ENTER)) {
      isWriting = 0;
      std::cout << "Ts pressed" << std::endl;
      rec = Recipe(rn, rd, rdate, ra);
    }

  }
  void reload(Recipe recipe) override {
    rn.clear();
    rd.clear();
    ra.clear();
    rdate.clear();
    rec = Recipe();
  }
};

class SlideMakerScene : public Scene {
private:
  Font font{};
  SceneManager* sm;
  std::string title, desc, image;
  unsigned int index;
  fs::path picdir;
  int whichWriting = 0;
  Recipe* rec;
public:
  SlideMakerScene(SceneManager& scm, Font fnt, unsigned int ind, fs::path picturedirectory) {
    sm = &scm;
    font = fnt;
    index = ind;
    picdir = picturedirectory;
  }

  void draw() override {
    ClearBackground(WHITE);
    std::string pthout = picdir;
    if (pthout.size() > 40) pthout.erase(pthout.begin(), pthout.end() - 40);

    DrawRectangleLinesEx(Rectangle(( w - MeasureTextEx(font, "WWWWWWWWWWWWWWWWWWWWWWWWWW", 40, 2).x ) / 2, 20, MeasureTextEx(font, "WWWWWWWWWWWWWWWWWWWWWWWWWW", 40, 2).x, 50), 3, BLACK);
    DrawTextEx(font, title.c_str(), (Vector2) {( w - MeasureTextEx(font, title.c_str(), 40, 2).x ) / 2, 25}, 40, 2, BLACK);
    if (title.empty()) DrawTextEx(font, "Title", (Vector2) {(w - MeasureTextEx(font, "Title", 40, 2).x) / 2, 25}, 40, 2, GRAY);

    DrawRectangleLinesEx(Rectangle(50, 175, w - 100, 50), 3, BLACK);
    DrawTextEx(font, pthout.c_str(), (Vector2) {6, 180}, 40, 2, BLACK);
    if (pthout.empty()) DrawTextEx(font, "Picture path", (Vector2) {60, 180}, 40, 2, GRAY);

    DrawRectangleLinesEx(Rectangle(50, 300, w - 100, 350), 3, BLACK);
    DrawTextEx(font, wrapping(desc, 120, 1600).c_str(), (Vector2) {(w - MeasureTextEx(font, wrapping(desc, 120, 1600).c_str(), 20, 2).x) / 2, h - 415}, 20, 2, BLACK);

    DrawRectangle(w-100, h - 60, 100, 60, BLUE);
    DrawRectangle(w - 90, h - 36, 50, 12, WHITE);
    DrawTriangle((Vector2) {w - 40, h - 48}, (Vector2) {w - 40, h - 12}, (Vector2) {w - 10, h - 30}, WHITE);

    DrawRectangle(w - 220, h - 60, 100, 60, BLUE);
    DrawText("END", w - 200 + ( 60 - MeasureText("END", 40) ) / 2, h - 46, 40, WHITE);
  }

  void input() override {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mp = GetMousePosition();
      if ((mp.y >= 20) && (mp.y <= 70) && (mp.x >= ( w - MeasureTextEx(font, "WWWWWWWWWWWWWWWWWWWWWWWWWW", 40, 2).x ) / 2) && (mp.x <= ( w - MeasureTextEx(font, "WWWWWWWWWWWWWWWWWWWWWWWWWW", 40, 2).x ) / 2 + MeasureTextEx(font, "WWWWWWWWWWWWWWWWWWWWWWWWWW", 40, 2).x)) whichWriting = 1;
      if ((mp.y >= 175) && (mp.y <= 225) && (mp.x >= 50) && (mp.x <= w - 100)) whichWriting = 2;
      if ((mp.y >= 350) && (mp.y <= 700) && (mp.x >= 50) && (mp.x <= w - 100)) whichWriting = 3;
      else whichWriting = 0;
      // if (())
    }
  }

  void reload(Recipe recipe) override {}
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
  Recipelist rl;
  load(rl);
  sm.add(std::make_unique<MenuScene>(rl, sm, montserrat));
  sm.add(std::make_unique<RecipeViewScene>(placeholdersalad, montserrat, sm));
  sm.add(std::make_unique<SlideScene>(placeholdersalad, montserrat, 0, sm));
  sm.add(std::make_unique<LoadScene>(sm, montserrat, rl));
  sm.add(std::make_unique<RecipeMakerScene>(montserrat, sm));
  sm.add(std::make_unique<SlideMakerScene>(sm, montserrat, 0, ""));
  sm.setCurrent((unsigned int) 5);
  SetExitKey(KEY_NULL);
  while (!WindowShouldClose()) {
    BeginDrawing();
    try {
      sm.input();
      sm.draw();
    }
    catch (const std::string& e) {
      std::cout << e << '\n';
    }
    EndDrawing();
  }
  CloseWindow();
}
