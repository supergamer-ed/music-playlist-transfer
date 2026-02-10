#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include <vector>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "music-playlist-transfer/libs/httplib.h"
#include "music-playlist-transfer/libs/nlohmann/json.hpp"
#include "music-playlist-transfer/spotifyFUNCS.hpp"

// size_t is an unsigned data type
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
  ((std::string*)userp)->append((char*)contents, size * nmemb); // got it from curl lib (not me)
  return size * nmemb; //returns the total size of the data being handled after the curl easy request
}

//creates the auth link for user to press 
std::string authorizationLink(const std::string new_state, std::string& new_uri){
  //ENCODED
  auto encoded_scope = curl_easy_escape(nullptr, "playlist-read-private playlist-modify-private", 0);
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(),0);
  auto encoded_state = curl_easy_escape(nullptr, new_state.c_str(), 0);
  new_uri = encoded_redirect_uri;
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

//needed for security measure 4 spotify state,  
//protection against attacks such as cross-site request forgery
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

//response for acquiring the api ke, once the user accepts and is redirected back to 127.0.0.1
void svrStarter(const std::string state, std::string& auth_code){
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

  svr.listen("127.0.0.1", 8080);
}

//acquires spotify token, will look into the refresh token probably
std::string spotifyTOKEN(std::string auth_code, std::string body){
  CURL *curl = curl_easy_init();
  std::string tokenBODY;
  if(curl){
    //headers
    struct curl_slist *headers_lis1 = NULL;
    headers_lis1 = curl_slist_append(headers_lis1, "Content-Type: application/x-www-form-urlencoded");
    //post setting options
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_lis1);
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

    //filter the writen data so it is easily parsed by nlohmann later (not if i use curlopt_header)
    int pos = tokenBODY.find("{"); // 01/2026 found out I used the wrong CURLOPT, shouldve used CURLOPT_HEADER
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

std::vector<Songs> spotify::getPlaylist(httplib::SSLClient& cli, int& tSong){
  std::string massDATA;
  auto res = cli.Get("/v1/me/playlists", [&](const char *data, size_t data_length) {
    massDATA.append(data, data_length); // Receive content with a content receiver on httplib git
    return true;
  });

  nlohmann::json parsedMASS = nlohmann::json::parse(massDATA);
  auto items = parsedMASS["items"];

  //options for the playlist will pop out using json pointers in nlohmann
  std::string playlistBASE;
  

  for(int i = 0; i < items.size(); i++){
    playlistBASE = "/items/" + std::to_string(i) + "/name";
    //starts to list the playlist, the parsedMASS.at gives the value of json at that path
    std::cout << std::to_string(i+1) + ". "
    << (parsedMASS.at(nlohmann::json::json_pointer(playlistBASE))).get<std::string>()
    << std::endl;
  }
  std::cout << "\nPlease input desired playlist: ";

  //CHECKS FOR INCORRECT INPUT, its on sight with them
  int options;
  std::cin >> options;
  while(std::cin.fail() || options > items.size() || options < 0){
    std::cin.clear();// clears the cin.fail flag
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //numeric_limits, gives u the limit of the of the primitive data, both min and max, streamsize will be the most 
    // or until it hits a new line
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
  tSong = (parsedMASS["total"]).get<int>();
  const int totalSONGS = tSong;
  std::vector<Songs> desiredPLAYLIST(totalSONGS);

  //putting songs into array below

  int songIndex = 0;
  offset = 0;
  std::string artist, title, artistP, titleP; //

  std::cout << std::endl << "getting them " << totalSONGS << " songs, gimme a min bro" << std::endl;
  
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

  std::cout << "\n Playlist Songs Include" << std::endl;
  for(int i = 0; i < tSong; i++){
    std::cout << (i+1) << ". " 
    << desiredPLAYLIST[i].title << " - " 
    << desiredPLAYLIST[i].author << std::endl;
  }

  return desiredPLAYLIST;
}

