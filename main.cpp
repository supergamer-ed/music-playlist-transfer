#include <iostream>
#include <curl/curl.h>
#include <string>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

int main(){
  CURL *curl;
  CURLcode res;
  std::string response;


  curl = curl_easy_init();
  if(curl){
    curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK){
      std::cerr << "CURL ERROR: " << curl_easy_strerror(res) << std::endl;
    }
    else{
      std::cout << "Response: \n" << response << std::endl;
    }
    curl_easy_cleanup(curl);
  }
  return 0;
}
