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

int main(){
  std::string auth_code, state, bodyDATA, encoded_URI;
  httplib::SSLClient cli("accounts.spotify.com");
  httplib::Result postRESULT;

  //spotify authorizationlink pops up
  std::cout << authorizationLink(state, encoded_URI) << std::endl;

  //should return the auth_code once its been given
  svrStarter(state, auth_code);

  //POST REQUEST BELOW TO SPOTIFY API

  //std::string jointID = CLIENT_ID + ":" + CLIENT_SECRET;
  //encodes the jointid and is set to true to make it URL passable
  //std::string encodedID = base64_encode(jointID, true);
  //there was a '.' that needed to be removed in order to post
  //encodedID[encodedID.size()-1] = '\0';

  /* hell nah jigsaw I put the secret and id in the
  params rather than in theheaders cuz it would not 
  accept it even when I maunally put them bothin a base64 encoder*/
  std::string body = "grant_type=authorization_code&code=" + auth_code + "&redirect_uri=" + encoded_URI + 
  "&client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET;
  std::cout << body << std::endl;
  std::string resultsPLZ = spotifyTOKEN(auth_code, body);
  std::cout << resultsPLZ << std::endl;
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
  //std::string headerID = "Authorization: Basic " + encodedID;
  if(curl){
    //headers
    struct curl_slist *headers_lis1 = NULL;
    headers_lis1 = curl_slist_append(headers_lis1, "Content-Type: application/x-www-form-urlencoded");
    //headers_lis1 = curl_slist_append(headers_lis1, headerID.c_str());
    //post setting options
    curl_easy_setopt(curl, CURLOPT_HEADER, headers_lis1);
    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(res != CURLE_OK){
      return "failed";
    }
    else{
      return "good";
    }
  }
  else{
    return "curl failed";
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
