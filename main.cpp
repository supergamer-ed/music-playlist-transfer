#include <cstdlib>
#include <fstream>
#include "music-playlist-transfer/spotifyFUNCS.hpp"
#include "music-playlist-transfer/youtubeFUNCS.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "music-playlist-transfer/libs/httplib.h"
#include "music-playlist-transfer/libs/nlohmann/json.hpp"
#include "music-playlist-transfer/songstruct.hpp"

int main(){
  std::string auth_code, bodyDATA, encoded_URI;
  std::string state = randomSTRING(16);
  httplib::SSLClient cli("api.spotify.com");
  static int totalSONGS = 0;


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
  tokensJSON = nlohmann::json::parse(tokenRESULTS);
  
  std::cout << "Getting Playlist" << std::endl;
  try{
  cli.set_bearer_token_auth(tokensJSON["access_token"].get<std::string>());
  }
  catch(...){
    std::cerr << "closing app, parsing error";
    return -1;
  }
  //returns
  std::vector<Songs> playlist = spotify::getPlaylist(cli, totalSONGS);
  
  std::cout << std::endl;

  std::cout << "----Now YT search portion----" << std::endl;
  /*
  std::string au;

  std::string st = randomSTRING(16);

  std::cout << "If anything use:\n" << ytauthlink(st) << std::endl;

  YT::svrStarter(st, au);

  httplib::SSLClient ytcli("oauth2.googleapis.com");
  httplib::SSLClient ytapicli("youtube.googleapis.com");

  std::string tokenYT = YT::code2TOKEN(au, ytcli);

  ytapicli.set_bearer_token_auth(tokenYT);
  */

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

  std::ofstream outputfile("testing.txt", std::ios::app);
  for(int i = 0; i < idytVIDS.size(); i++){
    outputfile << idytVIDS[i] << '\n';
  }

  outputfile.close();

  //---   end of spot->yt   ---//

}