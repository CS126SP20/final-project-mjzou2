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

// API key: RGAPI-b02b356e-064e-4b49-b67c-b9fdd06362ae

namespace myapp {

using cinder::app::KeyEvent;

size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream) {
  string data((const char*)ptr, (size_t)size * nmemb);
  *((stringstream*)stream) << data;
  return size * nmemb;
}

MyApp::MyApp() {}

void MyApp::setup() {
  read_buffer = download("https://na1.api.riotgames.com/lol/summoner/v4/summoners/by-name/b%27Qwacker%27?api_key=RGAPI-b02b356e-064e-4b49-b67c-b9fdd06362ae");

}

void MyApp::update() {}

void MyApp::draw() {
  const cinder::vec2 center = getWindowCenter();
  const cinder::ivec2 size = {500, 50};
  const cinder::Color color = cinder::Color::white();

  printText(read_buffer, color, size, center);
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

void MyApp::keyDown(KeyEvent event) {}

std::string MyApp::download(const std::string& url) {
  CURL* curl;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  /*
  curl_easy_setopt(
      curl, CURLOPT_NOSIGNAL,
      1);  // Prevent "longjmp causes uninitialized stack frame" bug
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
   */
  std::stringstream out;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  /* Perform the request, res will get the return code */
  res = curl_easy_perform(curl);
  /* Check for errors */
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    curl_easy_cleanup(curl);

    return out.str();
  }

}  // namespace myapp
}