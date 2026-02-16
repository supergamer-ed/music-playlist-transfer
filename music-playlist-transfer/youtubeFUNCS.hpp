#pragma once
#include <string>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "songstruct.hpp"

namespace YT{
    const std::string CLIENT_ID = "id";
    const std::string REDIRECT_URI = "http://127.0.0.1:8080/callback";
    const std::string CLIENT_SECRET = "secret";
    const std::string normalAPIkey = "api";// could be used for saerch vid
    void svrStarter(std::string state, std::string& auth_code);
    std::string code2TOKEN(std::string authCODE, httplib::SSLClient& cli);
    std::string createPlaylist(httplib::SSLClient& cli);
    std::string retrievePlayIDs(httplib::SSLClient& cli);
    void addtoPlaylist(const int posItem, std::vector<std::string>& arr);
    std::vector<std::string> searchVid(std::vector<Songs>& arr, const int opt);
    std::vector<std::string> retrieveItems(httplib::SSLClient& cli, std::string listID);
};

std::string ytauthlink(std::string& state);
