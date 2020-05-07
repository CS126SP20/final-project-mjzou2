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
#include <algorithm>
#include <map>

using std::stringstream;

const char kNormalFont[] = "Arial";

using cinder::Color;
using cinder::ColorA;
using cinder::Rectf;
using cinder::TextBox;
using cinder::app::KeyEvent;
using std::string;

string api_key = "RGAPI-d9a37c16-02bf-47f5-b79c-0530ece7b038";
string region = "NA1";


namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() {}

void MyApp::setup() {
  ImGui::initialize(ImGui::Options().font(getAssetPath("Arial.ttf"), 14));
  //ImGui::initialize();
  ImGui::StyleColorsClassic();
  icon_id = -1;
  res = false;
  std::ifstream ids_file;
  ids_file.open(cinder::app::getAssetPath("ids.json").c_str());
  ids_file >> champion_ids;
  std::ifstream champions_file;
  champions_file.open(cinder::app::getAssetPath("champion.json").c_str());
  nlohmann::json champions_full;
  champions_file >> champions_full;
  champions_data = champions_full["data"];
}

void MyApp::update() {
  first_rec_champ.clear();
  second_rec_champ.clear();
  third_rec_champ.clear();
  tags.clear();
}

void MyApp::draw() {
  cinder::gl::clear( Color( 0.1, 0.1, 0.1 ) );
  ImGuiIO &io = ImGui::GetIO();

  ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.3f), ImGuiCond_Always, ImVec2(0.5f,0.5f));
  ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.22f, io.DisplaySize.y * 0.3f), ImGuiCond_Always);

  ImGui::Begin("Input", 0, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration));

  ImGui::Text("Username");

  ImGui::SameLine();
  string buf;
  bool enter = ImGui::InputText("", &buf, ImGuiInputTextFlags_EnterReturnsTrue);
  if (enter) {
    username = buf;
    if (!username.empty()) {
      string response = getSummonerInfo(api_key, region, username);

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
        response = getRankedInfo(api_key, region, summoner_id);
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
          wins = solo_info["wins"].get<int>();
          losses = solo_info["losses"].get<int>();
          games = wins + losses;
          winrate = (double) wins / games;
        } else {
          tier = "UNRANKED";
          rank = "0";
          points = 0;
          wins = 0;
          losses = 0;
          games = 0;
          winrate = 0;
        }

        response = getMasteryInfo(api_key, region, summoner_id);
        nlohmann::json mastery_info = nlohmann::json::parse(response);
        first_champ_id = mastery_info.at(0)["championId"].get<int>();
        second_champ_id = mastery_info.at(1)["championId"].get<int>();
        third_champ_id = mastery_info.at(2)["championId"].get<int>();

        response = getCurrentGameInfo(api_key, region, summoner_id);

        nlohmann::json current_game_info = nlohmann::json::parse(response);

        in_game = status_code == 200;

        if (in_game) {
          current_game_time = current_game_info["gameLength"].get<int>();
          nlohmann::json participants = current_game_info["participants"];
          for (auto it = participants.begin(); it != participants.end(); ++it) {
            if (it.value()["summonerName"].get<string>() == username) {
              current_champ_id = (int) it.value()["championId"].get<long>();
            }
          }
        }

      }
    }
    enter = false;
  }
  if (!username.empty()) {
    //ImGui::Text("Username: %s", username.c_str());
    if (!res) {
      cinder::gl::color(cinder::Color::white());
      cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x, getWindowCenter().y - 150), 100);
    }
    const cinder::vec2 center = getWindowCenter();
    const cinder::ivec2 size = {500, 50};

    printText(username, Color( 0, 0, 0), 30, size, cinder::vec2(getWindowCenter().x, getWindowCenter().y - 150));


    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y + 350), 100);
    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x, getWindowCenter().y + 350), 100);
    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 350), 100);

    string first_champ_name = champion_ids[std::to_string(first_champ_id)];
    string second_champ_name = champion_ids[std::to_string(second_champ_id)];
    string third_champ_name = champion_ids[std::to_string(third_champ_id)];

    printText(first_champ_name, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x - 350, getWindowCenter().y + 385));
    printText(second_champ_name, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y + 385));
    printText(third_champ_name, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x - 250, getWindowCenter().y + 385));

    removeSpacesAndApostrophes(first_champ_name);
    removeSpacesAndApostrophes(second_champ_name);
    removeSpacesAndApostrophes(third_champ_name);

    string first_champ_tag1;
    string first_champ_tag2;
    string second_champ_tag1;
    string second_champ_tag2;
    string third_champ_tag1;
    string third_champ_tag2;

    first_champ_tag1 = champions_data[first_champ_name]["tags"].at(0).get<string>();
    tags.push_back(first_champ_tag1);
    if (champions_data[first_champ_name]["tags"].size() > 1) {
      first_champ_tag2 = champions_data[first_champ_name]["tags"].at(1).get<string>();
      tags.push_back(first_champ_tag2);
    }
    second_champ_tag1 = champions_data[second_champ_name]["tags"].at(0).get<string>();
    tags.push_back(second_champ_tag1);
    if (champions_data[second_champ_name]["tags"].size() > 1) {
      second_champ_tag2 = champions_data[second_champ_name]["tags"].at(1).get<string>();
      tags.push_back(second_champ_tag2);
    }

    third_champ_tag1 = champions_data[third_champ_name]["tags"].at(0).get<string>();
    tags.push_back(third_champ_tag1);

    if (champions_data[third_champ_name]["tags"].size() > 1) {
      third_champ_tag2 = champions_data[third_champ_name]["tags"].at(1).get<string>();
      tags.push_back(third_champ_tag2);
    }

    std::map<string, int> tag_frequencies;
    for (auto it = tags.begin(); it != tags.end(); ++it) {
      auto tag_it = tag_frequencies.find(*it);
      if (tag_it != tag_frequencies.end()) {
        tag_it->second++;
      } else {
        tag_frequencies.insert(std::make_pair(*it, 1));
      }
    }
    string most_freq;
    int freq = 0;
    for (auto it = tag_frequencies.begin(); it != tag_frequencies.end(); ++it) {
      if (it->second > freq) {
        freq = it->second;
        most_freq = it->first;
      }
    }
    tag_frequencies.erase(most_freq);
    string second_most_freq;
    freq = 0;
    for (auto it = tag_frequencies.begin(); it != tag_frequencies.end(); ++it) {
      if (it->second > freq) {
        freq = it->second;
        second_most_freq = it->first;
      }
    }

    // exact matches
    bool done = false;
    for (auto& it : champions_data.items())
    {
      string first_tag = it.value()["tags"].at(0).get<string>();
      string second_tag;
      if (it.value()["tags"].size() > 1) {
        second_tag = it.value()["tags"].at(1).get<string>();
      }
      if (most_freq == first_tag) {
        if (second_most_freq == second_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        }
      } else if (most_freq == second_tag) {
        if (second_most_freq == first_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        }
      }
    }

    // half matches
    if (!done) {
      for (auto& it : champions_data.items()) {
        string first_tag = it.value()["tags"].at(0).get<string>();
        string second_tag;
        if (it.value()["tags"].size() > 1) {
          second_tag = it.value()["tags"].at(1).get<string>();
        }
        if (most_freq == first_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        } else if (most_freq == second_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        } else if (second_most_freq == first_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        } else if (second_most_freq == second_tag) {
          string name = it.value()["name"].get<string>();
          if (name != first_champ_name && name != second_champ_name && name != third_champ_name) {
            if (first_rec_champ.empty()) {
              first_rec_champ = name;
            } else if (second_rec_champ.empty()) {
              second_rec_champ = name;
            } else if (third_rec_champ.empty()) {
              third_rec_champ = name;
              done = true;
              break;
            }
          }
        }
      }
    }

    cinder::Surface first_champ_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + first_champ_name + ".png")));
    cinder::Surface first_resized(50, 50, false);
    cinder::ip::resize(first_champ_tile, &first_resized);
    cinder::Rectf first_rect(getWindowCenter().x - 375, getWindowCenter().y + 325, getWindowCenter().x - 325, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(first_resized), first_rect);

    cinder::Surface second_champ_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + second_champ_name + ".png")));
    cinder::Surface second_resized(50, 50, false);
    cinder::ip::resize(second_champ_tile, &second_resized);
    cinder::Rectf second_rect(getWindowCenter().x - 325, getWindowCenter().y + 325, getWindowCenter().x - 275, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(second_resized), second_rect);

    cinder::Surface third_champ_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + third_champ_name + ".png")));
    cinder::Surface third_resized(50, 50, false);
    cinder::ip::resize(third_champ_tile, &third_resized);
    cinder::Rectf third_rect(getWindowCenter().x - 275, getWindowCenter().y + 325, getWindowCenter().x - 225, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(third_resized), third_rect);

    printText("TOP CHAMPIONS", Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y + 300));

    printText(first_rec_champ, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x - 50, getWindowCenter().y + 385));
    printText(second_rec_champ, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x, getWindowCenter().y + 385));
    printText(third_rec_champ, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 50, getWindowCenter().y + 385));

    removeSpacesAndApostrophes(first_rec_champ);
    removeSpacesAndApostrophes(second_rec_champ);
    removeSpacesAndApostrophes(third_rec_champ);

    cinder::Surface first_rec_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + first_rec_champ + ".png")));
    cinder::Surface first_rec_resized(50, 50, false);
    cinder::ip::resize(first_rec_tile, &first_rec_resized);
    cinder::Rectf first_rec_rect(getWindowCenter().x - 75, getWindowCenter().y + 325, getWindowCenter().x - 25, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(first_rec_resized), first_rec_rect);

    cinder::Surface second_rec_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + second_rec_champ + ".png")));
    cinder::Surface second_rec_resized(50, 50, false);
    cinder::ip::resize(second_rec_tile, &second_rec_resized);
    cinder::Rectf second_rec_rect(getWindowCenter().x - 25, getWindowCenter().y + 325, getWindowCenter().x + 25, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(second_rec_resized), second_rec_rect);

    cinder::Surface third_rec_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + third_rec_champ + ".png")));
    cinder::Surface third_rec_resized(50, 50, false);
    cinder::ip::resize(third_rec_tile, &third_rec_resized);
    cinder::Rectf third_rec_rect(getWindowCenter().x + 25, getWindowCenter().y + 325, getWindowCenter().x + 75, getWindowCenter().y + 375);
    cinder::gl::color(Color(1,1,1));
    cinder::gl::draw(cinder::gl::Texture2d::create(third_rec_resized), third_rec_rect);

    printText("RECOMMENDED CHAMPIONS", Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x, getWindowCenter().y + 300));

    if (in_game) {
      string current_champ_name = champion_ids[std::to_string(current_champ_id)];
      printText(current_champ_name, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 385));
      removeSpacesAndApostrophes(current_champ_name);
      cinder::Surface current_champ_tile(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/champion/" + current_champ_name + ".png")));
      cinder::Surface current_resized(50, 50, false);
      cinder::ip::resize(current_champ_tile, &current_resized);
      cinder::Rectf current_rect(getWindowCenter().x + 275, getWindowCenter().y + 325, getWindowCenter().x + 325, getWindowCenter().y + 375);
      cinder::gl::color(Color(1,1,1));
      cinder::gl::draw(cinder::gl::Texture2d::create(current_resized), current_rect);
      printText("CURRENT GAME", Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 300));
    } else {
      printText("Not In-Game", Color(0, 0, 0), 30, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 350));
    }

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
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y - 150), 100);

    const cinder::vec2 center = getWindowCenter();
    const cinder::ivec2 size = {500, 50};
    if (tier == "UNRANKED") {
      printText("Unranked", Color( 0, 0, 0), 30, size, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y - 150));
    } else {
      cinder::Surface current_emblem(cinder::loadImage(loadAsset("ranked-emblems/Emblem_" + tier + ".png")));
      cinder::Surface current_resized(100, 100, false);
      cinder::ip::resize(current_emblem, &current_resized);
      cinder::Rectf current_rect(getWindowCenter().x + 250, getWindowCenter().y - 210, getWindowCenter().x + 350, getWindowCenter().y - 110);
      cinder::gl::color(Color(1,1,1));
      cinder::gl::draw(cinder::gl::Texture2d::create(current_resized), current_rect);
      string current_full_rank;
      if (rank == "0") {
        current_full_rank = tier + " " + std::to_string(points) + " LP";
      } else {
        current_full_rank = tier + " " + rank + " " + std::to_string(points) + " LP";
      }
      printText("CURRENT RANK", Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y - 220));
      printText(current_full_rank, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y - 90));

      cinder::gl::color(cinder::Color::white());
      cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 100), 100);
      string predictive_full_rank = calculatePeak(rank, tier, games, winrate);
      cinder::Surface predictive_emblem(cinder::loadImage(loadAsset("ranked-emblems/Emblem_" + predictive_tier + ".png")));
      cinder::Surface predictive_resized(100, 100, false);
      cinder::ip::resize(predictive_emblem, &predictive_resized);
      cinder::Rectf predictive_rect(getWindowCenter().x + 250, getWindowCenter().y + 40, getWindowCenter().x + 350, getWindowCenter().y + 140);
      cinder::gl::color(Color(1,1,1));
      cinder::gl::draw(cinder::gl::Texture2d::create(predictive_resized), predictive_rect);
      printText("PREDICTED PEAK", Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 30));
      printText(predictive_full_rank, Color(0, 0, 0), 15, {500, 50}, cinder::vec2(getWindowCenter().x + 300, getWindowCenter().y + 160));

      cinder::gl::color(cinder::Color::white());
      cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x, getWindowCenter().y + 100), 100);
      string winrate_str = std::to_string(winrate).substr(2, 3);
      winrate_str.insert(2, ".");
      printText(winrate_str + "% Win Ratio", Color(0, 0, 0), 30, {500, 50}, cinder::vec2(getWindowCenter().x, getWindowCenter().y + 100));

      cinder::gl::color(cinder::Color::white());
      cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y + 100), 100);
      printText(std::to_string(games) + " Games", Color(0, 0, 0), 30, {500, 50}, cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y + 100));
    }
  }
  if (icon_id != -1 && !res) {
    cinder::gl::color(cinder::Color::white());
    cinder::gl::drawSolidCircle(cinder::vec2(getWindowCenter().x - 300, getWindowCenter().y - 150), 100);
    cinder::Surface icon(cinder::loadImage(loadAsset("dragontail-10.9.1/10.9.1/img/profileicon/" + std::to_string(icon_id) + ".png")));
    cinder::Surface resized(100, 100, false);
    cinder::ip::resize(icon, &resized);

    cinder::Rectf drawRect(getWindowCenter().x - 350, getWindowCenter().y - 200, getWindowCenter().x - 250, getWindowCenter().y - 100);
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
  status_code = response.status_code;
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
  removeSpacesAndApostrophes(name);
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

string MyApp::getMasteryInfo(string api_key, string region, string summoner_id) {
  return call("https://" + region + ".api.riotgames.com/lol/champion-mastery/v4/champion-masteries/by-summoner/" + summoner_id + "?api_key=" + api_key);
}

string MyApp::getCurrentGameInfo(string api_key, string region, string summoner_id) {
  return call("https://" + region + ".api.riotgames.com/lol/spectator/v4/active-games/by-summoner/" + summoner_id + "?api_key=" + api_key);
};

string MyApp::calculatePeak(string rank, string tier, int games, int winrate) {
  if (tier == "CHALLENGER") {
    predictive_tier = "CHALLENGER";
    return "CHALLENGER";
  }
  int rank_int;
  if (rank == "I") {
    rank_int = 1;
  } else if (rank == "II") {
    rank_int = 2;
  } else if (rank == "III") {
    rank_int = 3;
  } else if (rank == "IV") {
    rank_int = 4;
  }
  Tier tier_enum = stringToTier(tier);
  if (tier == "MASTER" || tier == "GRANDMASTER") {
    if (winrate >= 0.54) {
      tier_enum = Tier::CHALLENGER;
    } else if (winrate >= 0.52) {
      tier_enum = static_cast<Tier>(tier_enum + 1);
    } else {
      if (games < 100) {
        tier_enum = Tier::CHALLENGER;
      } else if (games < 300) {
        tier_enum = static_cast<Tier>(tier_enum + 1);
      }
    }
  } else {
    if (winrate >= 0.8) {
      if (games < 20) {
        rank_int -= 4;
      } else if (games < 50) {
        rank_int -= 5;
      } else if (games < 100) {
        rank_int -= 9;
      } else {
        rank_int -= 12;
      }
    } else if (winrate >= 0.7) {
      if (games < 20) {
        rank_int -= 3;
      } else if (games < 50) {
        rank_int -= 4;
      } else if (games < 100) {
        rank_int -= 7;
      } else {
        rank_int -= 10;
      }
    } else if (winrate >= 0.6) {
      if (games < 20) {
        rank_int -= 2;
      } else if (games < 100) {
        rank_int -= 3;
      } else {
        rank_int -= 5;
      }
    } else if (winrate >= 0.56) {
      if (games < 50) {
        rank_int -= 2;
      } else if (games < 100) {
        rank_int -= 3;
      } else {
        rank_int -= 2;
      }
    } else if (winrate >= 0.52) {
      if (games < 100) {
        rank_int -= 2;
      } else {
        rank_int -= 1;
      }
    } else {
      if (games < 20) {
        rank_int -= 2;
      } else if (games < 100) {
        rank_int -= 1;
      }
    }
  }
  if (tier_enum >= 7) {
    predictive_tier = tierToString(tier_enum);
    return predictive_tier;
  } else {
    int tiers_up = 0;
    while (rank_int < 1) {
      rank_int += 4;
      tiers_up++;
    }
    tier_enum = static_cast<Tier>(tier_enum + tiers_up);
    string tier_string = tierToString(tier_enum);
    string rank_string;
    if (rank_int == 1) {
      rank_string = "I";
    } else if (rank_int == 2) {
      rank_string = "II";
    } else if (rank_int == 3) {
      rank_string = "III";
    } else if (rank_int == 4) {
      rank_string = "IV";
    }
    predictive_tier = tier_string;
    return tier_string + " " + rank_string;
  }
}

MyApp::Tier MyApp::stringToTier(string tier) {
  Tier tier_enum;
  if (tier == "CHALLENGER") {
    tier_enum = Tier::CHALLENGER;
  } else if (tier == "GRANDMASTER") {
    tier_enum = Tier::GRANDMASTER;
  } else if (tier == "MASTER") {
    tier_enum = Tier::MASTER;
  } else if (tier == "DIAMOND") {
    tier_enum = Tier::DIAMOND;
  } else if (tier == "PLATINUM") {
    tier_enum = Tier::PLATINUM;
  } else if (tier == "GOLD") {
    tier_enum = Tier::GOLD;
  } else if (tier == "SILVER") {
    tier_enum = Tier::SILVER;
  } else if (tier == "BRONZE") {
    tier_enum = Tier::BRONZE;
  } else if (tier == "IRON") {
    tier_enum = Tier::IRON;
  } else if (tier == "UNRANKED") {
    tier_enum = Tier::UNRANKED;
  }
  return tier_enum;
}

string MyApp::tierToString(Tier tier) {
  string tier_string;
  if (tier == Tier::CHALLENGER) {
    tier_string = "CHALLENGER";
  } else if (tier == Tier::GRANDMASTER) {
    tier_string = "GRANDMASTER";
  } else if (tier == Tier::MASTER) {
    tier_string = "MASTER";
  } else if (tier == Tier::DIAMOND) {
    tier_string = "DIAMOND";
  } else if (tier == Tier::PLATINUM) {
    tier_string = "PLATINUM";
  } else if (tier == Tier::GOLD) {
    tier_string = "GOLD";
  } else if (tier == Tier::SILVER) {
    tier_string = "SILVER";
  } else if (tier == Tier::BRONZE) {
    tier_string = "BRONZE";
  } else if (tier == Tier::IRON) {
    tier_string = "IRON";
  } else if (tier == Tier::UNRANKED) {
    tier_string = "UNRANKED";
  }
  return tier_string;
}

void MyApp::removeSpacesAndApostrophes(string& input) {
  string x = " ", y = "";
  size_t pos;
  while ((pos = input.find(x)) != std::string::npos) {
    input.replace(pos, 1, y);
  }
  for (int i = 1; i < input.size(); i++) {
    if (input[i - 1] == '\'')
      input[i] = std::tolower(input[i]);
  }
  string i = "'", j = "";
  size_t pos2;
  while ((pos2 = input.find(i)) != std::string::npos) {
    input.replace(pos2, 1, j);
  }
}
}  // namespace myapp

