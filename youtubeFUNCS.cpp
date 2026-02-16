#include <iostream>
#include <random>
#include <curl/curl.h>
#include "nlohmann/json.hpp"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "youtubeFUNCS.hpp"


std::string ranSTR(int len){
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

std::string ytauthlink(std::string& state){
    state = ranSTR(16);
    auto encoded_scope = curl_easy_escape(nullptr,"https://www.googleapis.com/auth/youtube",0);
    auto encoded_redir = curl_easy_escape(nullptr, "http://127.0.0.1:8080/callback", 0);
    auto encoded_clientid = curl_easy_escape(nullptr, YT::CLIENT_ID.c_str(), 0);
    auto encoded_state = curl_easy_escape(nullptr, state.c_str(), 0);

    std::string clientID = encoded_clientid;
    std::string redirURL = encoded_redir;
    std::string scope = encoded_scope;
    state = encoded_state;

    curl_free(encoded_clientid);
    curl_free(encoded_redir);
    curl_free(encoded_scope);
    curl_free(encoded_state);

    std::string authLINK = "https://accounts.google.com/o/oauth2/v2/auth";
    authLINK += "?scope=" + scope
             + "&client_id=" + clientID
             + "&redirect_uri=" + redirURL
             + "&include_granted_scopes=true&response_type=code&access_type=offline"
             + "&state=" + state;

    return authLINK;
}

void YT::svrStarter(std::string state, std::string& auth_code){
  httplib::Server svr;
  // I was following the wrong doc, I need to request for code rather than token
  // anything after # in http is not sent to the server (something that was occuring in my app)
  svr.Get("/callback", [&, state](const httplib::Request &req, httplib::Response &res){
      if(state == req.get_param_value("state")){
        if(req.has_param("code")){
          auth_code = req.get_param_value("code");
          res.set_content("Hold on", "text/plain");
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

std::string YT::code2TOKEN(std::string authCODE, httplib::SSLClient& cli){
  std::string token;

  httplib::Params ytparams =
  {{"client_secret", YT::CLIENT_SECRET.c_str()},
  {"client_id", YT::CLIENT_ID.c_str()},
  {"code", authCODE.c_str()},
  {"grant_type", "authorization_code"},
  {"redirect_uri", YT::REDIRECT_URI.c_str()}};

  nlohmann::json j;

  auto yt_R = cli.Post("/token", ytparams);
  if(yt_R->status == 200){
    try{
      j = nlohmann::json::parse(yt_R->body);
      token = j["access_token"];
    }
    catch(nlohmann::json::parse_error& ex){
      std::cerr << "parse error at byte " << ex.byte << std::endl;
    }

  }
  else{
    std::cerr << yt_R->body << "\ntoken has no value!";
  }

  return token;
}

std::string YT::createPlaylist(httplib::SSLClient& cli){
  std::string namePlay;
  int maxCHAR = 140, option;
  std::cin.ignore();
  std::cout << "Enter name for Playlist: ";
  std::getline(std::cin, namePlay);


  if(namePlay.length() > 140){
    namePlay.resize(maxCHAR);
  }

  nlohmann::json body = {
    {"snippet", {
        {"title", namePlay}, 
        {"description", "spot2YT"}
      }
    }
  };


  std::cout << "1. Private\n2. Public\nSelect option: ";
  std::cin >> option;

  while(option < 1 || option > 2 || std::cin.fail()){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max());
    std::cout << "invalid, enter a valid integer: ";
    std::cin >> option;
  }

  switch(option){
    case 1:
    body["status"]["privacyStatus"] = "private";
    break;
    case 2:
    body["status"]["privacyStatus"] = "public";
    break;
  }


  auto res = cli.Post("/youtube/v3/playlists?part=snippet%2Cstatus", body.dump(), "application/json");
  
  if(res->status == 200){
    std::cout << "Successfully created the playlist, " << namePlay << "!!"<< std::endl;
    try{
      nlohmann::json jsondata = nlohmann::json::parse(res->body);
      return jsondata["id"];
    }
    catch(...){
      std::cerr << "Error in parasing playlist id in create playlist" << std::endl;
      return "";
    }
  }
  else{
    std::cerr << "Insuccessful, error code: " << res->status << "  " << res->body << std::endl;
    return "";  
  }
}

std::string YT::retrievePlayIDs(httplib::SSLClient& cli){
  std::string url = "/youtube/v3/playlists?part=snippet,contentDetails,id&maxResults=50&mine=true";
  nlohmann::json jsonbody;
  std::string body;

  auto res = cli.Get(url,
  [&](const char *data, size_t data_length) {
    body.append(data, data_length);
    return true;
  });

  try{
    jsonbody = nlohmann::json::parse(body);
  }
  catch(...){
    std::cerr << "error in parsing";//ultimately will make them create a new one
  }

  auto items = jsonbody["items"].size();
  for(int i = 0; i < items; i++){
    std::cout << (i+1) << ". " 
    << jsonbody["items"][i]["snippet"]["title"].dump() << std::endl;
  }

  std::cout << "Choose an option above: ";
  int opt = -1;
  std::cin >> opt;
  while(std::cin.fail()|| opt < 1 || opt > items){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "try again, invalid integer, choose an option: ";
    std::cin >> opt;
  }
  opt--;

  return jsonbody["items"][opt]["id"];
}

void YT::addtoPlaylist(const int posItem, std::vector<std::string>& arr){
  std::string randomSTR = ranSTR(16), auth_code;
  httplib::SSLClient apicli("youtube.googleapis.com"), authcli("oauth2.googleapis.com");
  
  std::cout << "Click on:\n" << ytauthlink(randomSTR) << std::endl;
  
  //obtains auth code, next will exhcnage for token
  YT::svrStarter(randomSTR, auth_code);

  std::string tokenBEAR = YT::code2TOKEN(auth_code, authcli);

  apicli.set_bearer_token_auth(tokenBEAR);

  int option;
  std::cout << "1. Create Playlist\n2. Insert into exisiting\nChoose an option: ";
  std::cin >> option;

  while(std::cin.fail() || option < 1 || option > 2){
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    std::cout << "Try agian, enter valid option: ";
    std::cin >> option;    
  }


  std::string playID;
  switch (option)
  {
  case 1:
    playID = YT::createPlaylist(apicli);
    break;
    case 2:
    playID = YT::retrievePlayIDs(apicli);
    break;
    default:
    return;
  }
  std::cout << "Hold on while it inserts the songs" << std::endl;
  int max = arr.size();
  httplib::Result res;
  std::string urlendpoint = "/youtube/v3/playlistItems?part=snippet";
  for(int i = posItem; i < max;i++){
    nlohmann::json jsonDATA = {
      {"snippet", {
        {"playlistId", playID},
        {"resourceId", {
          {"kind", "youtube#video"},
          {"videoId", arr[i]}
        }}
      }}
    };

    res = apicli.Post(urlendpoint, jsonDATA.dump(), "application/json");
    if(res->status != 200){
      std::cout << "error encountered" << std::endl;
      if(res->status == 403){
        std::cout << "Try again tomorrow, max quotas" << std::endl;
      }
      else{
        std::cout << res->status << std::endl << res->body << std::endl;
        return;
      }
    }
  }

  std::cout << "Check playlist now :)" << std::endl;

  
}

std::vector<std::string> YT::searchVid(std::vector<Songs>& arrS, const int option){
  //this is the get url that will be passed, will add q whihc is the song title
  httplib::SSLClient cli("youtube.googleapis.com");
  std::string url = "/youtube/v3/search?part=snippet,id&maxResults=3&type=video&topicId=/m/04rlf&order=relevance&key="+
                    YT::normalAPIkey + "&q=";
  std::string body, holder;
  nlohmann::json jsonbody;
  int num;
  if(arrS.size() - option > 100){
    num = 100;
  }
  else{
    num = arrS.size() - option;
  }


  std::vector<std::string> arr(num);
  

  for(int i = option, j = 0; i < arrS.size(); i++, j++){
    body.clear();
    std::string encodedtitle = httplib::detail::encode_url(arrS[i].title);
    std::string newURL = url + encodedtitle;
    auto res = cli.Get(newURL, [&](const httplib::Response &response) {
      if(response.status != 200){
        return false;
      }
      return true;
    },
    [&](const char *data, size_t data_length){
      body.append(data, data_length);
      return true;});
    if(res && res->status == 200){
      try{
        jsonbody = nlohmann::json::parse(body);
        if(jsonbody["items"].size() != 0){
          holder = jsonbody["items"][0]["id"]["videoId"];
          arr[j] = holder;
        }
      }
      catch(...){
        std::cerr << "Parasing error for song " << (i+1) << std::endl;
        continue;
      }
    }
    else if(res && res->status != 200){
      if(res->status == 403){
        std::cerr << "Quota Maxed, try again tomorrow";
        break;
      }
      else
        std::cerr << "Status Error code: " << res->status << " with the following: \n" << res->body; 
      continue;
    }
  }

  std::cout << "Success, woooohooooo" << std::endl;
  
  return arr;
}

std::vector<std::string> YT::retrieveItems(httplib::SSLClient& cli, std::string listID){
  std::string body;
  nlohmann::json jsonbody;
  std::string id = httplib::detail::encode_url(listID);
  std::string url = "/youtube/v3/playlistItems?part=snippet&maxResults=5&playlistId=" +
  id;
  int index = 0;
  std::vector<std::string> arr; //the author correlates with the channel name so it does not matter, just need title
  arr.reserve(1000);//change number if u like but barely anyone has that many songs

  auto res = cli.Get(url,
  [&](const httplib::Response &response) {
    if(response.status != 200){
      std::cout << "Error Code: " << response.status << "\nMeans: " 
      << response.body << std::endl;
      return false;
    }
    else return true;
  },
  [&](const char *data, size_t data_length){
    body.append(data, data_length);
    return true;
  });
  

  //due to the page refresh in yt, kinda lit ngl
  while(res && res->status == 200){
    try{
      jsonbody = nlohmann::json::parse(body);
    }
    catch(...){
      std::cerr << "Error Parsing";
      return arr;
    }


    for(int i = 0; i < jsonbody["items"].size(); i++){
      arr.push_back(jsonbody["items"][i]["snippet"]["title"]);
    }

    if(jsonbody.contains("nextPageToken")){
      body.clear();
      std::string nextPT = jsonbody["nextPageToken"];
      std::string newURL = url + "&pageToken=" + nextPT;
      res = cli.Get(newURL,
        [&](const httplib::Response &response) {
          if(response.status != 200){
            std::cout << "Error Code: " << response.status << "\nMeans: " 
            << response.body << std::endl;
            return false;
          }
          else return true;
        },
        [&](const char *data, size_t data_length){
          body.append(data, data_length);
          return true;
        });
    }
    else break;
  }

  return arr;
}

