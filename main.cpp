#include <cstdlib>
#include <fstream>
#include "spotifyFUNCS.hpp"
#include "youtubeFUNCS.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "songstruct.hpp"

void clearSCREEN();

void spotifyTOyoutube();

void youtubeTOspotify();

void insertSONGS();

int main(){
  int choiceOption;
  std::cout << "1.Spotify -> YT\n2.YT->Spotify\n3.Insert Song Id list\nEnter an option: ";
  std::cin >> choiceOption;

  while(std::cin.fail() || choiceOption < 1 || choiceOption > 3){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max());
    std::cout << "Try again, Enter a valid integer: ";
    std::cin >> choiceOption;
  }


  clearSCREEN();

  switch(choiceOption){
    case 1:
    spotifyTOyoutube();
    break;
    case 2:
    youtubeTOspotify();
    break;
    case 3:
    insertSONGS();
    break;
  }

}

void spotifyTOyoutube(){
  std::string auth_code, bodyDATA, encoded_URI;
  std::string state = randomSTRING(16);
  httplib::SSLClient cli("api.spotify.com");
  int totalSONGS = 0;


  #if defined(_WIN32)
        std::string link = "start " + authorizationLink(state, encoded_URI);
  #elif defined(__APPLE__)
        std::string link = "open \"" + authorizationLink(state, encoded_URI) + "\"";
  #else
        std::string link = "xdg-open \"" + authorizationLink(state, encoded_URI) + "\"";
  #endif

  std::system(link.c_str());

  std::cout << "If anything use:\n" << authorizationLink(state, encoded_URI) << std::endl;

  //should return the auth_code once its been given
  svrStarter(state, auth_code);

  //-----POST REQUEST BELOW TO SPOTIFY API------//
  
  //compose to make a post request using curl
  std::string body1 = "grant_type=authorization_code&code=" + auth_code + "&redirect_uri=" + encoded_URI +
  "&client_id=" + CLIENT_ID + "&client_secret=" + CLIENT_SECRET;

  nlohmann::json tokensJSON;
  std::string tokenRESULTS = spotifyTOKEN(auth_code, body1);

  clearSCREEN();

  std::cout << "Getting Playlist" << std::endl;

  try{
  tokensJSON = nlohmann::json::parse(tokenRESULTS);
  cli.set_bearer_token_auth(tokensJSON["access_token"].get<std::string>());
  }
  catch(...){
    std::cerr << "closing app, parsing error";
    return;
  }

  //returns
  std::vector<Songs> playlist = spotify::getPlaylist(cli, totalSONGS);
  
  std::cout << std::endl;

  std::cout << "----Now YT search portion----" << std::endl;

  std::cout << "\nNote due to the limit of yt, u will have to open tomorrow sad :(, if > 100";
  std::cout << "\nChoose an option to start the search, look above at the printed songs: ";
  
  int option;
  std::cin >> option;
  while(option < 1 || option > totalSONGS || std::cin.fail()){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    std::cout << "try again, enter a valid integer: ";
    std::cin >> option;
  }
  option--; // corresponds correclty with the array position elments
  std::vector<std::string> idytVIDS = YT::searchVid(playlist, option);

  std::ofstream outputfile("songID.txt", std::ios::app);
  for(int i = 0; i < idytVIDS.size(); i++){
    outputfile << idytVIDS[i] << '\n';
  }
  outputfile.close();

  //---   end of spot->yt   ---//
}

void youtubeTOspotify(){
  httplib::SSLClient apicli("youtube.googleapis.com"), authcli("oauth2.googleapis.com");
  std::string auth, randSTR = randomSTRING(16);

  std::cout << "Click on:\n" << ytauthlink(randSTR) << std::endl;
  YT::svrStarter(randSTR, auth);
  
  clearSCREEN();

  std::string tokenYT = YT::code2TOKEN(auth, authcli);
  apicli.set_bearer_token_auth(tokenYT);

  std::string playlistID = YT::retrievePlayIDs(apicli);

  std::vector<std::string> playlistSongs = YT::retrieveItems(apicli, playlistID);

  clearSCREEN();

  if(playlistSongs.empty()){
    std::cout << "Empty, choose another" << std::endl;
    while(playlistSongs.empty()){
      playlistID = YT::retrievePlayIDs(apicli);
      playlistSongs = YT::retrieveItems(apicli, playlistID);
    }
    clearSCREEN();
  }


  std::cout << "------Songs------" << std::endl;
  for(int i = 0; i < playlistSongs.size(); i++){
    std::cout << (i+1) << ". " << playlistSongs[i] << std::endl;
  }

  spotify::searchItemsNStore(playlistSongs);

  std::cout << "Done" << std::endl;
  return;
  //end of yt->spot
}

void insertSONGS(){
  clearSCREEN();
  std::string nameFILE;
  std::vector<std::string> arr;

  std::cout << "Enter the name of the file containing of ID's, (make sure it in the same place as the executable)";
  std::getline(std::cin, nameFILE, '\n');
  nameFILE = "../" + nameFILE + ".txt";

  std::ifstream file(nameFILE.c_str());
  
  while(!file.is_open()){
    nameFILE.clear();
    file.clear();
    std::cout << "could not find file, try again or place inside folder (where main.cpp is) and relaunch by crtl+c\nEnter file name:";
    std::getline(std::cin, nameFILE, '\n');
    nameFILE = "../" + nameFILE + ".txt";
    file.open(nameFILE.c_str());
  }

  std::string s;
  while(file >> s){
    arr.push_back(s);
  }
  clearSCREEN();

  int max = arr.size(), choiceOption;
  std::cout << "From what song would u like to start from? or try again tomorrow and note it: ";
  std::cin >> choiceOption;

  while(std::cin.fail() || choiceOption < 1 || choiceOption > max){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    std::cout << "Try agian, enter valid option: ";
    std::cin >> choiceOption;    
  }
  choiceOption--;//corrects array indexing
  
  YT::addtoPlaylist(choiceOption, arr);

}

void clearSCREEN(){
  #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
