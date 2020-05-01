// Copyright (c) 2020 [Your Name]. All rights reserved.

#include "my_app.h"

#include <cinder/app/App.h>
#include <cinder/Color.h>
#include <cinder/gl/gl.h>
#include <cinder/Font.h>
#include <cinder/Text.h>
#include <cinder/Vector.h>
#include <cinder/gl/draw.h>
#include <cinder/gl/gl.h>

#include <string>
#include <iostream>
#include <sstream>

using std::stringstream;

const char kNormalFont[] = "Arial";

using cinder::Color;
using cinder::ColorA;
using cinder::Rectf;
using cinder::TextBox;
using cinder::app::KeyEvent;
using std::string;

// API key: RGAPI-2780f235-d7db-4796-8a91-2000b43b310f
// https://na1.api.riotgames.com/lol/summoner/v4/summoners/by-name/b%27Qwacker%27?api_key=RGAPI-2780f235-d7db-4796-8a91-2000b43b310f


namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() {}

void MyApp::setup() {
  ImGui::initialize();
}

void MyApp::update() {}

void MyApp::draw() {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {500, 50};
  const cinder::Color color = cinder::Color::white();
  
  // printText("Test", color, size, center);
  cinder::gl::clear( Color( 0, 0, 0 ) );
  string buf = "";
  bool enter = ImGui::InputText("Username", &buf, ImGuiInputTextFlags_EnterReturnsTrue);
  username = buf;
  if (enter) {
    if (!username.empty()) {
      string response = getSummonerInfo("RGAPI-2780f235-d7db-4796-8a91-2000b43b310f", "NA1", username);
      nlohmann::json j = nlohmann::json::parse(response);
      string temp = j["accountId"];
      id = temp;
    }
    enter = false;
  }
  ImGui::Text("%s", id.c_str());
}

template <typename C>
void MyApp::printText(const string& text, const C& color,
                      const cinder::ivec2& size, const cinder::vec2& loc) {
  cinder::gl::color(color);

  auto box = TextBox()
                 .alignment(TextBox::CENTER)
                 .font(cinder::Font(kNormalFont, 30))
                 .size(size)
                 .color(color)
                 .backgroundColor(ColorA(0, 0, 0, 0))
                 .text(text);

  const auto box_size = box.getSize();
  const cinder::vec2 locp = {loc.x - box_size.x / 2, loc.y - box_size.y / 2};
  const auto surface = box.render();
  const auto texture = cinder::gl::Texture::create(surface);
  cinder::gl::draw(texture, locp);
}

void MyApp::keyDown(KeyEvent event) {
}

std::string MyApp::call(const std::string& url) {
  auto response = cpr::Get(cpr::Url{url});
  return response.text;
}
string MyApp::getSummonerInfo(string api_key, string region, string name) {
  return call("https://" + region + ".api.riotgames.com/lol/summoner/v4/summoners/by-name/" + name + "?api_key=" + api_key);
}
string MyApp::getRankedInfo(string api_key, string region, string summoner_id) {
  return call("https://" + region + ".api.riotgames.com/lol/league/v4/entries/by-summoner/" + summoner_id + "?api_key=" + api_key);
}
string MyApp::getMatchList(string api_key, string region, string account_id,
                           string queue, string begin_time, string begin_index,
                           string end_index) {
  return call("https://" + region + ".api.riotgames.com/lol/match/v4/matchlists/by-account/" + account_id + "?beginIndex=" + begin_index + "&endIndex=" + end_index + "&beginTime=" + begin_time + "&queue=" + queue + "&api_key=" + api_key);
}
string MyApp::getMatchInfo(string api_key, string region, string match_id) {
  return call("https://" + region + ".api.riotgames.com/lol/match/v4/matches/" + match_id + "?api_key=" + api_key);
}

}  // namespace myapp

