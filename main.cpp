#include <iostream>
#include <string>
#include <random>
#include <fstream>
#include <curl/curl.h>
#include <thread>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "libs/base64.h"
#include "libs/nlohmann/json.hpp"

//variables used in all URLs
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secret";
std::string const redirect_uri = "http://localhost:5000/callback";

std::string randomSTRING(int len);
///sss

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
  authURL += "?client_id=" + CLIENT_ID
          + "&response_type=code"
          + "&redirect_uri=" + new_uri
          + "&show_dialog=true"
          + "&scope=" + scope
          + "&state=" + new_state;

  return authURL;
}

std::string spotifyTOKEN(std::string auth_code, httplib::Headers& headers);

void svrStarter(std::string state, std::string& auth_code);

int main(){
  std::string auth_code, state, bodyDATA;
  httplib::SSLClient cli("accounts.spotify.com");
  httplib::Headers headers;
  httplib::Result postRESULT;
  
  std::thread svr(svrStarter);
  std::cout << authorizationLink(state) << std::endl; //if std::endl is removed, it will not display this std::cout
  svrStarter(state, auth_code);
  svr.join();

  std::cout << auth_code << "1";
  std::cout << auth_code << "2" << std::endl;


}

void svrStarter(std::string state, std::string& auth_code){
  httplib::Server svr;

  //obtain code for api, next will request token
  svr.Get("/callback", [&, state](const httplib::Request &req, httplib::Response &res){
    if(state == req.get_param_value("state")){
      if(req.has_param("code")){
        auth_code = req.get_param_value("code");
        res.set_content("HELLO", "text/plain");
        std::cout << "HOLD ON" << std::endl;
        return;
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

  svr.listen("localhost", 5000);
}

std::string spotifyTOKEN(std::string auth_code, httplib::Headers& headers){
  std::string base_auth = CLIENT_ID + ":" + CLIENT_SECRET;

  //encoding URL
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(), 0);
  std::string new_redirectURI(encoded_redirect_uri);
  curl_free(encoded_redirect_uri);

  std::string dataPOST = "code=" + auth_code
                       + "&grant_type=authorization_code"
                       + "&redirect_uri" + new_redirectURI;


  
  std::string b64HE = base64_encode(reinterpret_cast<const unsigned char*>(base_auth.c_str()), base_auth.length());

  return dataPOST;
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

