#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include "libs/nlohmann/json.hpp"
#include "libs/httplib.h"
#include "libs/base64.h"

//variables used in all URLs
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secret";
std::string const redirect_uri = "http://localhost:5000/callback";

std::string randomSTRING(int len);

std::string authorizationLink(std::string& new_state){
  std::string state = randomSTRING(16);
  //ENCODED
  auto encoded_scope = curl_easy_escape(nullptr, "playlist-read-private playlist-modify-private", 0);
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(),0);
  auto encoded_state = curl_easy_escape(nullptr, state.c_str(), 0);
  std::string new_uri(encoded_redirect_uri);
  new_state = encoded_state;
  std::string scope(encoded_scope);
  curl_free(encoded_state);
  curl_free(encoded_redirect_uri);
  curl_free(encoded_scope);


  std::string authURL = "https://accounts.spotify.com/authorize";
  authURL = "?client_id=" + CLIENT_ID
          + "&response_type=code"
          + "&redirect_uri=" + new_uri
          + "&show_dialog=true"
          + "&scope=" + scope
          + "&state=" + new_state;

  return authURL;
}

std::string spotifyTOKEN(std::string auth_code);

int main(){
  std::string auth_code, state;
  httplib::Server svr;
  httplib::Headers headers;


  std::cout << authorizationLink(state) << std::endl; //if std::endl is removed, it will not display this std::cout

  //obtain code for api, next svr.get will request token
  svr.Get("/callback", [&auth_code, state](const httplib::Request &req, httplib::Response &res){
    if(state == req.get_param_value("state")){
      if(req.has_param("code")){
        auth_code = req.get_param_value("code");
        res.set_content("HELLO", "text/plain");
        std::cout << "HOLD ON" << std::endl;
      }
      if(req.has_param("error")){
        auth_code = req.get_param_value("error");
        res.set_content("ERROR: " + auth_code, "text/plain");
        std::cout << "ERROR: " << auth_code << std::endl;
      }
    }
  });


  svr.listen("localhost", 5000);
}

std::string spotifyTOKEN(std::string auth_code){
  std::string req_body = "https://accounts.spotify.com/api/token";
  std::string base_auth = CLIENT_ID + ":" + CLIENT_SECRET;

  //encoding URL
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(), 0);
  std::string new_redirectURI(encoded_redirect_uri);
  curl_free(encoded_redirect_uri);

  req_body += "?code=" + auth_code + "&grant_type=authorization_code";
            + "&redirect_uri=" + new_redirectURI;
    
  httplib::Headers headers = {{"content-type", "application/x-www-form-urlencoded"},
                              {"Authorization", "Basic " + 
                              base64_encode(reinterpret_cast<const unsigned char*>(auth_code.c_str()), auth_code.length())}
                              //interprets the pointer data type to a const unsigned char and takes c string which is y .c_str is used
                            };
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

