#include <iostream>
#include <curl/curl.h>
#include <string>
#include "libs/nlohmann/json.hpp"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string playlistcall(char urlLink[]){
  CURL *curl;
  CURLcode res;
  std::string response;
  nlohmann::json j= {
    "song_name", "artist" 
  };

  curl = curl_easy_init();
  if(curl){
    curl_easy_setopt(curl, CURLOPT_URL, urlLink);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK){
      std::cerr << "CURL ERROR: " << curl_easy_strerror(res) << std::endl;
    }
    curl_easy_cleanup(curl);
  }
  return response;
}

int main(){
  char uInput[100];
  std::cout << "Playlist: ";
  std::cin >> uInput;
  std::cout << playlistcall(uInput);
  

}
