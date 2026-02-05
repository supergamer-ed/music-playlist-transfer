#include <iostream>
#include <random>
#include <curl/curl.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "libs/music-playlist-transfer/youtubeFUNCS.hpp"

std::string ranSTR(int len){
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

std::string ytauthlink(std::string& state){
    state = ranSTR(16);
    auto encoded_scope = curl_easy_escape(nullptr,"https://www.googleapis.com/auth/youtube",0);
    auto encoded_redir = curl_easy_escape(nullptr, "http://127.0.0.1:8080/callback", 0);
    auto encoded_clientid = curl_easy_escape(nullptr, YT::CLIENT_ID.c_str(), 0);
    auto encoded_state = curl_easy_escape(nullptr, state.c_str(), 0);

    std::string clientID = encoded_clientid;
    std::string redirURL = encoded_redir;
    std::string scope = encoded_scope;
    state = encoded_state;

    curl_free(encoded_clientid);
    curl_free(encoded_redir);
    curl_free(encoded_scope);
    curl_free(encoded_state);

    std::string authLINK = "https://accounts.google.com/o/oauth2/v2/auth";
    authLINK += "?scope=" + scope
             + "&client_id=" + clientID
             + "&redirect_uri=" + redirURL
             + "&include_granted_scopes=true&response_type=code&access_type=offline"
             + "&state=" + state;

    return authLINK;
}

void YT::svrStarter(std::string state, std::string& auth_code){
  httplib::Server svr;
  // I was following the wrong doc, I need to request for code rather than token
  // anything after # in http is not sent to the server (something that was occuring in my app)
  svr.Get("/callback", [&, state](const httplib::Request &req, httplib::Response &res){
      if(state == req.get_param_value("state")){
        if(req.has_param("code")){
          auth_code = req.get_param_value("code");
          res.set_content("Second Part, Hold on", "text/plain");
          std::cout << "HOLD ON" << std::endl;
          if(!auth_code.empty()){
            svr.stop();
          }
        }
        else if(req.has_param("error")){
          auth_code = req.get_param_value("error");
          res.set_content("ERROR: " + auth_code, "text/plain");
          std::cout << "ERROR: " << auth_code << std::endl;
        }
    }
    else {
      svr.stop();
      std::cout << "bruh security error, just relaunch";
    }
  });

  svr.listen("127.0.0.1", 8080);
}


