// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <cinder/app/App.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <cinder/Text.h>
#include "CinderImGui.h"
#include <cinder/ip/Resize.h>
#include <vector>


using std::string;
using std::vector;

namespace myapp {

class MyApp : public cinder::app::App {
 public:
  MyApp();
  void setup() override;
  void update() override;
  void draw() override;

  enum Tier {
    UNRANKED = 0,
    IRON = 1,
    BRONZE = 2,
    SILVER = 3,
    GOLD = 4,
    PLATINUM = 5,
    DIAMOND = 6,
    MASTER = 7,
    GRANDMASTER = 8,
    CHALLENGER = 9
  };
 private:
  template <typename C>
  void printText(const std::string& text, const C& color, int font_size, const cinder::ivec2& size,
                 const cinder::vec2& loc);
  string call(const std::string& url);
  string getSummonerInfo(string api_key, string region, string name);
  string getRankedInfo(string api_key, string region, string summoner_id);
  string getMatchList(string api_key, string region, string account_id, string queue, string begin_index, string end_index);
  string getMatchInfo(string api_key, string region, string match_id);
  string getMasteryInfo(string api_key, string region, string summoner_id);
  string calculatePeak(string rank, string tier, int games, int winrate);
  Tier stringToTier(string tier);
  string tierToString(Tier tier);
  void removeSpacesAndApostrophes(string& input);
  string username;
  string account_id;
  string summoner_id;
  string rank;
  string tier;
  int points;
  int icon_id;
  bool res;
  int wins;
  int losses;
  int games;
  double winrate;
  string predictive_tier;
  int first_champ_id;
  int second_champ_id;
  int third_champ_id;
  nlohmann::json champion_ids;
  nlohmann::json champions_data;
  vector<string> tags;
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
