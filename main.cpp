#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include "libs/httplib.h"


std::string randomSTRING(int len);

std::string authorizationLink(){
  std::string CLIENT_ID = "secrets";
  std::string redirect_uri = "http://localhost:5000/callback";
  std::string state = randomSTRING(16);
  //ENCODED
  auto encoded_scope = curl_easy_escape(nullptr, "playlist-read-private playlist-modify-private", 0);
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(),0);
  auto encoded_state = curl_easy_escape(nullptr, state.c_str(), 0);
  std::string new_uri(encoded_redirect_uri);
  std::string new_state(encoded_state);
  std::string scope(encoded_scope);
  curl_free(encoded_state);
  curl_free(encoded_redirect_uri);
  curl_free(encoded_scope);


  std::string authURL = "https://accounts.spotify.com/authorize";
  authURL += "?client_id=" + CLIENT_ID;
  authURL += "&response_type=code";
  authURL += "&redirect_uri=" + new_uri;
  authURL += "&show_dialog=true";
  authURL += "&scope=" + scope;
  authURL += "&state=" + new_state;

  return authURL;
}

std::string getSpotifytoken();


int main(){
  httplib::Server svr;
  std::cout << authorizationLink() << std::endl;


  svr.Get("/callback", [](const httplib::Request &req, httplib::Response &res){
    if(req.has_param("code") && !(req.has_param("error"))){
      std::string codeSPO = req.get_param_value("code");
      res.set_content("?code=" + codeSPO, "text/plain");
    }
    else{
      std::string error_SPOT = req.get_param_value("error");
      res.set_content("error =" + error_SPOT, "text/plain");
    }
  });
  


  svr.listen("localhost", 5000);
}

std::string randomSTRING(int len){
  const std::string CHARS
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv"
          "wxyz0123456789";

  // Create a random number generator
  std::random_device rd;
  std::mt19937 generator(rd());

  std::uniform_int_distribution<> distribution(0, CHARS.size()-1);

  std::string random_STR;
  for(int i = 0; i < len; ++i){
    random_STR += CHARS[distribution(generator)];
  }
  return random_STR;
}

