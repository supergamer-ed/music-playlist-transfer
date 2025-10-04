#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include <thread>
#include <fstream>
#include <vector>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "libs/base64.h"
#include "libs/nlohmann/json.hpp"

//variables used in all URLs
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secrets";
std::string const redirect_uri = "http://localhost:5000/callback";

struct Songs{
  std::string title;
  std::string author;
};

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

//(return type) spotifysongFILTER(std::string massDATA); probably idk

int main(){
  std::string auth_code, state, bodyDATA, encoded_URI;
  httplib::SSLClient cli("api.spotify.com");
  
  //spotify authorizationlink pops up
  std::cout << authorizationLink(state, encoded_URI) << std::endl;

  //should return the auth_code once its been given
  svrStarter(state, auth_code);

  //POST REQUEST BELOW TO SPOTIFY API

  /* hell nah jigsaw I put the secret and id in the
  params rather than in the headers cuz it would not 
  accept it even when I maunally put them bothin a base64 encoder*/
  std::string body1 = "grant_type=authorization_code&code=" + auth_code + "&redirect_uri=" + encoded_URI + 
  "&client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET;

  std::string tokenRESULTS = spotifyTOKEN(auth_code, body1);
  nlohmann::json tokensJSON = nlohmann::json::parse(tokenRESULTS);

  std::cout << "Getting Playlist" << std::endl;
  cli.set_bearer_token_auth(tokensJSON["access_token"].get<std::string>());
  // calling json with the value wanted then using .get(), allows the value obtained as a string rather than an
  // object of json
  
  std::string massDATA;
  auto res = cli.Get("/v1/me/playlists",
  [&](const char *data, size_t data_length) {
    massDATA.append(data, data_length); // Receive content with a content receiver on httplib git
    return true;
  });

  nlohmann::json parsedMASS = nlohmann::json::parse(massDATA);
  auto items = parsedMASS["items"];

  //options for the playlist will pop out using json pointers in nlohmann
  std::string playlistBASE;
  int options = 0;
  std::cout << "\nPlease input desired playlist: " << std::endl;
  
  while(options < items.size()){
    playlistBASE = "/items/" + std::to_string(options) + "/name";
    //starts to list the playlist, the parsedMASS.at gives the value of json at that path 
    std::cout << std::to_string(options+1) + ". "
    << (parsedMASS.at(nlohmann::json::json_pointer(playlistBASE))).get<std::string>() 
    << std::endl;
    options++;
  }
  std::cin >> options;

  //CHECKS FOR INCORRECT INPUT, its on sight with them
  while(options > items.size() || 0 > options){
    std::cout << "Invalid, choose a valid number" << std::endl;
    std::cin >> options;
  }

  //obtain the path of desired playlist because it displays all playlists
  playlistBASE = "/" + std::to_string(options-1) + "/tracks/href";
  massDATA = (items.at(nlohmann::json::json_pointer(playlistBASE))).get<std::string>();
  int pos = massDATA.find("v"); // was a unique character
  playlistBASE = massDATA.substr(pos-1);
  int offset = 0;
  std::string playlistPATH = playlistBASE + "?limit=100&market=US&offset=" + std::to_string(offset);

  //1st GETrequest
  massDATA.clear();
  res = cli.Get(playlistPATH, [&](const char *data, size_t data_length) {
    massDATA.append(data, data_length); // Receive content with a content receiver on httplib git
    return true;
  });
  
  //store the beginning of the massDATA obtained into array
  pos = massDATA.find("{");
  massDATA = massDATA.substr(pos);
  parsedMASS = nlohmann::json::parse(massDATA); //going to make a more ordered json sheet to store song name and artist
  int num = (parsedMASS["total"]).get<int>();
  const int totalSONGS = num;
  std::vector<Songs> desiredPLAYLIST(totalSONGS);
  
  //putting songs into array below

  int songIndex = 0;
  offset = 0;
  std::string artist, title, artistP, titleP;

  std::cout << "getting them " << totalSONGS << " songs, gimme a min bro" << std::endl;
  playlistPATH.clear();
  while(offset < totalSONGS){
    playlistPATH = playlistBASE + "?limit=100&market=US&offset=" + std::to_string(offset);

    massDATA.clear();
    auto res = cli.Get(playlistPATH, [&](const char *data, size_t data_length) {
      massDATA.append(data, data_length);
      return true;
    });

    int pos = massDATA.find("{");
    massDATA = massDATA.substr(pos);
    auto parasedNEW = nlohmann::json::parse(massDATA);
    auto itemsPN = parasedNEW["items"];

    //using item.size() actually gives u the total amount of songs, good since its not always 100
    for(int i = 0; i < itemsPN.size(); i++){
      titleP = "/items/" + std::to_string(i) + "/track/name";
      artistP = "/items/" + std::to_string(i) + "/track/artists/0/name";

      title = parasedNEW.at(nlohmann::json::json_pointer(titleP)).get<std::string>();
      artist = parasedNEW.at(nlohmann::json::json_pointer(artistP)).get<std::string>();

      if (songIndex < totalSONGS) {
        desiredPLAYLIST[songIndex].title = title;
        desiredPLAYLIST[songIndex].author = artist;
        songIndex++; 
      }
    }

    offset += 100;
  }

  std::cout << "\n Playlist Songs" << std::endl;
  for(int i = 0; i < songIndex; i++){
    std::cout << desiredPLAYLIST[i].title << " - " << desiredPLAYLIST[i].author << std::endl; 
  }
  //if u want to make it into a txt file, just do the loop into that fstream file and import it into urs or whatever
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
  //stackoverflow coming in clutch with this bad boi
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
