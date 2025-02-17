#include <iostream>
#include <string>
#include <curl/curl.h>
#include "libs/httplib.h"

std::string authorizationLink(){
  std::string CLIENT_ID = "ssksks";
  std::string redirect_uri = "http://localhost:5000/callback";
  std::string scope = "playlist-read-private";
  auto encoded_redirect_uri = curl_easy_escape(nullptr, redirect_uri.c_str(),0);
  std::string new_uri(encoded_redirect_uri);
  curl_free(encoded_redirect_uri);

  std::string authURL = "https://accounts.spotify.com/authorize";
  authURL += "?client_id=" + CLIENT_ID;
  authURL += "&response_type=code";
  authURL += "&redirect_uri=" + new_uri;
  authURL += "&show_dialog=true";
  authURL += "&scope=" + scope;

  return authURL;
}


int main(){
  httplib::Server svr;

  std::cout << authorizationLink() << std::endl;
  svr.Get("/callback", [](const httplib::Request &req, httplib::Response &res){
    res.set_content("Hello World!", "text/plain");
  });


  svr.listen("localhost", 5000);
}