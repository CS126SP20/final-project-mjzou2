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
  ImGui::initialize(ImGui::Options().font(getAssetPath("Arial.ttf"), 14));
  //ImGui::initialize();
  ImGui::StyleColorsClassic();
  icon_id = -1;
  res = false;
}

void MyApp::update() {}

void MyApp::draw() {
  cinder::gl::clear( Color( 0.1, 0.1, 0.1 ) );
  ImGuiIO &io = ImGui::GetIO();

  ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.3f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
  ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.3f, io.DisplaySize.y * 0.3f), ImGuiCond_Always);

  ImGui::Begin("Input", 0, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration));

  ImGui::Text("Username");

  ImGui::SameLine();
  string buf;
  bool enter = ImGui::InputText("", &buf, ImGuiInputTextFlags_EnterReturnsTrue);

  for (int i = 0; i < 3; i++) {
    ImGui::NewLine();
  }

  if (enter) {
    username = buf;
    if (!username.empty()) {
      string response = getSummonerInfo("RGAPI-2780f235-d7db-4796-8a91-2000b43b310f", "NA1", username);

      if (std::isdigit(response[0])) {
        printText(response, Color(0, 0, 0), 40, {500, 50}, getWindowCenter());
        ImGui::End();
        username = response;
        res = true;
        return;
      }
      res = false;

      nlohmann::json summoner_info = nlohmann::json::parse(response);

      account_id = summoner_info["accountId"].get<std::string>();
      summoner_id = summoner_info["id"].get<std::string>();
      username = summoner_info["name"].get<std::string>();
      icon_id = summoner_info["profileIconId"].get<int>();

      if (!summoner_id.empty()) {
        response = getRankedInfo("RGAPI-2780f235-d7db-4796-8a91-2000b43b310f", "NA1", summoner_id);
        nlohmann::json ranked_info = nlohmann::json::parse(response);

        if (!ranked_info.empty()) {
          nlohmann::json solo_info = ranked_info.at(0);
          if (solo_info["queueType"].get<std::string>() == "RANKED_FLEX_SR" &&
              ranked_info.size() > 1) {
            solo_info = ranked_info.at(1);
          }
          tier = solo_info["tier"].get<std::string>();
          if (tier == "CHALLENGER" || tier == "GRANDMASTER" || tier == "MASTER") {
            rank = "0";
          } else {
            rank = solo_info["rank"].get<std::string>();
          }
          points = solo_info["leaguePoints"].get<int>();
        } else {
          tier = "UNRANKED";
          rank = "0";
          points = 0;
        }

        response = getMatchList("RGAPI-2780f235-d7db-4796-8a91-2000b43b310f", "NA1", account_id, "420", "0", "100");
        nlohmann::json matches = nlohmann::json::parse(response);
        
      }
    }
    enter = false;
  }
  if (!username.empty()) {
    //ImGui::Text("Username: %s", username.c_str());
    if (!res) {
      cinder::gl::color(cinder::Color::white());
      cinder::gl::drawSolidCircle(getWindowCenter(), 100);
    }
    const cinder::vec2 center = getWindowCenter();
    const cinder::ivec2 size = {500, 50};

    printText(username, Color( 0, 0, 0), 40, size, center);
  }
  /*
  if (!account_id.empty()) {
    ImGui::Text("Account ID: %s", account_id.c_str());
  }
  if (!summoner_id.empty()) {
    ImGui::Text("Summoner ID: %s", summoner_id.c_str());
  }
   */
  if (!rank.empty() && !res) {
    //ImGui::Text("Rank: %s", rank.c_str());
    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y), 100);

    const cinder::vec2 center = cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y);
    const cinder::ivec2 size = {500, 50};
    if (tier == "UNRANKED") {
      printText("Unranked", Color( 0, 0, 0), 40, size, center);
    } else {
      cinder::Surface emblem(cinder::loadImage(loadAsset("ranked-emblems/Emblem_" + tier + ".png")));

      cinder::Surface resized(100, 100, false);

      cinder::ip::resize(emblem, &resized);

      cinder::Rectf drawRect(getWindowCenter().x + 250, getWindowCenter().y - 60, getWindowCenter().x + 350, getWindowCenter().y + 40);
      cinder::gl::color(Color(1,1,1));
      cinder::gl::draw(cinder::gl::Texture2d::create(resized), drawRect);
      string full_rank;
      if (rank == "0") {
        full_rank = tier + " " + std::to_string(points) + " LP";
      } else {
        full_rank = tier + " " + rank + " " + std::to_string(points) + " LP";
      }
      printText(full_rank, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 60));
    }
  }
  if (icon_id != -1 && !res) {
    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y), 100);
    cinder::Surface icon(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/profileicon/" + std::to_string(icon_id) + ".png")));
    cinder::Surface resized(100, 100, false);
    cinder::ip::resize(icon, &resized);

    cinder::Rectf drawRect(getWindowCenter().x - 350, getWindowCenter().y - 50, getWindowCenter().x - 250, getWindowCenter().y + 50);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(resized), drawRect);
  }

  ImGui::End();
}

template <typename C>
void MyApp::printText(const string& text, const C& color, int font_size,
                      const cinder::ivec2& size, const cinder::vec2& loc) {
  cinder::gl::color(color);

  auto box = TextBox()
                 .alignment(TextBox::CENTER)
                 .font(cinder::Font(kNormalFont, font_size))
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

std::string MyApp::call(const std::string& url) {
  auto response = cpr::Get(cpr::Url{url});
  int status_code = response.status_code;
  if (status_code == 200) {
    return response.text;
  } else if (status_code == 400) {
    return "400 (bad request)";
  } else if (status_code == 401) {
    return "401 (unauthorized)";
  } else if (status_code == 403) {
    return "403 (forbidden)";
  } else if (status_code == 404) {
    return "404 (data not found)";
  } else if (status_code == 405) {
    return "405 (method not allowed)";
  } else if (status_code == 415) {
    return "415 (unsupported media type)";
  } else if (status_code == 429) {
    return "429 (rate limit exceeded)";
  } else if (status_code == 500) {
    return "500 (internal server error)";
  } else if (status_code == 503) {
    return "503 (service unavailable)";
  } else if (status_code == 504) {
    return "504 (gateway timeout)";
  } else {
    return string();
  }
}
string MyApp::getSummonerInfo(string api_key, string region, string name) {
  string x = " ", y = "%20";
  size_t pos;
  while ((pos = name.find(x)) != std::string::npos) {
    name.replace(pos, 1, y);
  }
  return call("https://" + region + ".api.riotgames.com/lol/summoner/v4/summoners/by-name/" + name + "?api_key=" + api_key);
}
string MyApp::getRankedInfo(string api_key, string region, string summoner_id) {
  return call("https://" + region + ".api.riotgames.com/lol/league/v4/entries/by-summoner/" + summoner_id + "?api_key=" + api_key);
}
string MyApp::getMatchList(string api_key, string region, string account_id,
                           string queue, string begin_index,
                           string end_index) {
  return call("https://" + region + ".api.riotgames.com/lol/match/v4/matchlists/by-account/" + account_id + "?beginIndex=" + begin_index + "&endIndex=" + end_index + "&queue=" + queue + "&api_key=" + api_key);
}
string MyApp::getMatchInfo(string api_key, string region, string match_id) {
  return call("https://" + region + ".api.riotgames.com/lol/match/v4/matches/" + match_id + "?api_key=" + api_key);
}

}  // namespace myapp

