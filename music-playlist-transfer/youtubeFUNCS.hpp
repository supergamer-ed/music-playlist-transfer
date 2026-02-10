#pragma once
#include <string>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "songstruct.hpp"

namespace YT{
    const std::string CLIENT_ID = "id";
    const std::string REDIRECT_URI = "http://127.0.0.1:8080/callback";
    const std::string CLIENT_SECRET = "secret";
    const std::string normalAPIkey = "apikey";// could be used for saerch vid
    void svrStarter(std::string state, std::string& auth_code);
    std::string code2TOKEN(std::string authCODE, httplib::SSLClient& cli);
    void createPlaylist(httplib::SSLClient& cli);
    std::string retrievePlayIDs(httplib::SSLClient& cli);
    void addtoPlaylist(httplib::SSLClient& cli, const std::string token, const int posItem, const std::string playID);
    std::vector<std::string> searchVid(std::vector<Songs>& arr, const int opt);
};

std::string ytauthlink(std::string& state);
