// Copyright (c) 2020 CS126SP20. All rights reserved.

#ifndef FINALPROJECT_APPS_MYAPP_H_
#define FINALPROJECT_APPS_MYAPP_H_

#include <cinder/app/App.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <cinder/Text.h>
#include "CinderImGui.h"

using std::string;

namespace myapp {

class MyApp : public cinder::app::App {
 public:
  MyApp();
  void setup() override;
  void update() override;
  void draw() override;
  void keyDown(cinder::app::KeyEvent) override;
  template <typename C>
  void printText(const std::string& text, const C& color, const cinder::ivec2& size,
                 const cinder::vec2& loc);
  string call(const std::string& url);
  string getSummonerInfo(string api_key, string region, string name);
  string getRankedInfo(string api_key, string region, string summoner_id);
  string getMatchList(string api_key, string region, string account_id, string queue, string begin_time, string begin_index, string end_index);
  string getMatchInfo(string api_key, string region, string match_id);
  string username;
  string id;
};

}  // namespace myapp

#endif  // FINALPROJECT_APPS_MYAPP_H_
