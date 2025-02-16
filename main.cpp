#include <iostream>
#include <curl/curl.h>
#include <string>
#include "libs/httplib.h"

// could put in main prob
std::string authorizationLink(){
  CURL *curl = curl_easy_init();

  std::string CLIENT_ID = "CLIENT ID";
  std::string state = "houndiin";
  char redirect_uri[] = "http://localhost:5000/callback";
  char scope[] = "playlist-read-private";
  
  char *encoded_scope = curl_easy_escape(curl, scope, 0);
  char *encoded_redirectUri = curl_easy_escape(curl, redirect_uri, 0);

  //URL AUTHORIZATION
  std::string authorization_url = "https://accounts.spotify.com/authorize?"
       "client_id=" + CLIENT_ID + "&response_type=code" +
       "&redirect_uri=" + encoded_redirectUri +  "&scope=" + encoded_scope +
      "&state=" + state;
  curl_easy_cleanup(curl);
  return authorization_url;
}

std::string getSpotifyToken();

int main(){
  std::cout << authorizationLink() << std::endl;
  

  return 0;
}