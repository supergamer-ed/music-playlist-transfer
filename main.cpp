#include <cstdlib>
#include "libs/music-playlist-transfer/spotifyFUNCS.hpp"
#include "libs/music-playlist-transfer/youtubeFUNCS.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "libs/nlohmann/json.hpp"

struct Songs{
  std::string title;
  std::string author;
};

int main(){
  std::string auth_code, state, bodyDATA, encoded_URI;
  httplib::SSLClient cli("api.spotify.com");


  #if defined(_WIN32)
        std::string link = "start " + authorizationLink(state, encoded_URI);
  #elif defined(__APPLE__)
        std::string link = "open \"" + authorizationLink(state, encoded_URI) + "\"";
  #else
        std::string link = "xdg-open \"" + authorizationLink(state, encoded_URI) + "\"";
  #endif

  std::cout << "If anything use:\n" << authorizationLink(state, encoded_URI) << std::endl;


  //should return the auth_code once its been given
  svrStarter(state, auth_code);

  //-----POST REQUEST BELOW TO SPOTIFY API------//
  
  //compose to make a post request using curl
  std::string body1 = "grant_type=authorization_code&code=" + auth_code + "&redirect_uri=" + encoded_URI +
  "&client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET;

  std::string tokenRESULTS = spotifyTOKEN(auth_code, body1);
  nlohmann::json tokensJSON = nlohmann::json::parse(tokenRESULTS);

  std::cout << "Getting Playlist" << std::endl;
  cli.set_bearer_token_auth(tokensJSON["access_token"].get<std::string>());
  // calling json with the value wanted then using .get(), allows the value obtained as a string rather than an
  // object of json

  
  //obtains playlist availables
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
  std::cout << "\nPlease input desired playlist: " << std::endl;

  for(int i = 0; i < items.size(); i++){
    playlistBASE = "/items/" + std::to_string(i) + "/name";
    //starts to list the playlist, the parsedMASS.at gives the value of json at that path
    std::cout << std::to_string(i+1) + ". "
    << (parsedMASS.at(nlohmann::json::json_pointer(playlistBASE))).get<std::string>()
    << std::endl;
  }

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
  int num = (parsedMASS["total"]).get<int>();
  const int totalSONGS = num;
  std::vector<Songs> desiredPLAYLIST(totalSONGS);

  //putting songs into array below

  int songIndex = 0;
  offset = 0;
  std::string artist, title, artistP, titleP; //

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

  std::cout << "\n Playlist Songs Include" << std::endl;
  for(int i = 0; i < songIndex; i++){
    std::cout << desiredPLAYLIST[i].title << " - " << desiredPLAYLIST[i].author << std::endl;
  }

  std::cout << "----Now YT portion----" << std::endl;
  
  std::string st, au;

  std::cout << ytauthlink(st) << std::endl;


  YT::svrStarter(st, au);


  std::cout << au << std::endl;

  httplib::SSLClient ytcli("oauth2.googleapis.com");

  httplib::Params ytparams =
  {{"client_secret", YT::CLIENT_SECRET.c_str()},
  {"client_id", YT::CLIENT_ID.c_str()},
  {"code", au.c_str()},
  {"grant_type", "authorization_code"},
  {"redirect_uri", YT::REDIRECT_URI.c_str()}};

  auto yt_R = ytcli.Post("/token", ytparams);
  if(yt_R->status == 200){
    std::cout << yt_R->body;
  }
  else{
    std::cerr << yt_R->body; 
  }

}