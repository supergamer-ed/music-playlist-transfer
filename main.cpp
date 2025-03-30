#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include <thread>
#include <fstream>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "libs/base64.h"
#include "libs/nlohmann/json.hpp"

//variables used in all URLs
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secret";
std::string const redirect_uri = "http://localhost:5000/callback";

std::string randomSTRING(int len);

std::string authorizationLink(std::string& new_state, std::string& new_uri){
  std::string state = randomSTRING(16);
  //ENCODED
  auto encoded_scope = curl_easy_escape(nullptr, "playlist-read-private playlist-modify-private", 0);
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(),0);
  auto encoded_state = curl_easy_escape(nullptr, state.c_str(), 0);
  new_uri = encoded_redirect_uri;
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

std::string spotifyTOKEN(std::string auth_code, std::string body);

void svrStarter(std::string state, std::string& auth_code);

// going to place this writeback function in a future main.cpp since im spliting these apis into header files
// size_t is an unsigned data type 
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
  ((std::string*)userp)->append((char*)contents, size * nmemb); // got it from curl lib (not me)
  return size * nmemb; //returns the total size of the data being handled after the curl easy request
}

int main(){
  std::string auth_code, state, bodyDATA, encoded_URI;
  
  //spotify authorizationlink pops up
  std::cout << authorizationLink(state, encoded_URI) << std::endl;

  //should return the auth_code once its been given
  svrStarter(state, auth_code);

  //POST REQUEST BELOW TO SPOTIFY API

  /* hell nah jigsaw I put the secret and id in the
  params rather than in theheaders cuz it would not 
  accept it even when I maunally put them bothin a base64 encoder*/
  std::string body = "grant_type=authorization_code&code=" + auth_code + "&redirect_uri=" + encoded_URI + 
  "&client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET;

  std::string tokenRESULTS = spotifyTOKEN(auth_code, body);
  nlohmann::json tokensJSON = nlohmann::json::parse(tokenRESULTS);
  std::cout << std::endl;
  std::cout << tokenRESULTS;
  std::cout << std::endl;
  std::cout << tokensJSON["access_token"] << std::endl;
  std::cout << tokensJSON["refresh_token"] << std::endl;
  std::cout << std::endl;
  

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

  svr.listen("localhost", 5000);
}

std::string spotifyTOKEN(std::string auth_code, std::string body){
  CURL *curl = curl_easy_init();
  std::string tokenBODY;
  if(curl){
    //headers
    struct curl_slist *headers_lis1 = NULL;
    headers_lis1 = curl_slist_append(headers_lis1, "Content-Type: application/x-www-form-urlencoded");
    //post setting options
    curl_easy_setopt(curl, CURLOPT_HEADER, headers_lis1);
    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // need this as curl needs to know how to handle the data
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &tokenBODY);

    CURLcode res = curl_easy_perform(curl);
    //curl cleanup
    curl_slist_free_all(headers_lis1);
    curl_easy_cleanup(curl);

    //filter the writen data so it is easily parsed by nlohmann later
    int pos = tokenBODY.find("{");
    tokenBODY = tokenBODY.substr(pos); // takes alway the unneccessary curl info

    if(res != CURLE_OK){
      return "RESULT ERROR";
    }
    else{
      return tokenBODY;
    }
  }
  else{
    return "CURL ERROR";
  }
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
