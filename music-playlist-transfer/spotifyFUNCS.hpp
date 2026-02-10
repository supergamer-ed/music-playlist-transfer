#pragma once
#include <iostream>
#include <string>
#include <random>
#include <curl/curl.h>
#include <vector>
#include <cstdlib>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "libs/httplib.h"
#include "songstruct.hpp"


//constants
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secret";
std::string const redirect_uri = "http://127.0.0.1:8080/callback";

//look at spotifyFUNCS.cpp for more info

std::string randomSTRING(int len);

void svrStarter(const std::string state, std::string& auth_code);

std::string authorizationLink(const std::string new_state, std::string& new_uri);

std::string spotifyTOKEN(std::string auth_code, std::string body);

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

namespace spotify{
    std::vector<Songs> getPlaylist(httplib::SSLClient& pcli, int& totalSONGS);
};

