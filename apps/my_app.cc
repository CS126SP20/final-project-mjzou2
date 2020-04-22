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

const char kNormalFont[] = "Arial";

using cinder::Color;
using cinder::ColorA;
using cinder::Rectf;
using cinder::TextBox;
using cinder::app::KeyEvent;
using std::string;


namespace myapp {

using cinder::app::KeyEvent;

MyApp::MyApp() { }

void MyApp::setup() {
  mTime = cinder::app::getElapsedSeconds();
  mCurl = HttpClient::make();

  HttpRequest get;
  get.mUrl = "https://na1.api.riotgames.com/lol/summoner/v4/summoners/by-name/Qwacker?api_key=RGAPI-de515e2a-d7c7-4d55-894a-8afdac2fb59a";
  get.mMethod = HTTP_GET;
  get.mCallback = [=](HttpResponse* response, HttpClient* curl) { std::printf("Request complete with code %ld; headers are %s\n", response->mResponseCode, curl->jsonToString(response->mHeaders).c_str()); };
  mCurl->addRequest(get);
}

void MyApp::update() { }

void MyApp::draw() {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {500, 50};
  const cinder::Color color = cinder::Color::white();

  PrintText("Test", color, size, center);
}

template <typename C>
void MyApp::PrintText(const string& text, const C& color, const cinder::ivec2& size,
               const cinder::vec2& loc) {
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

void MyApp::keyDown(KeyEvent event) { }

}  // namespace myapp
