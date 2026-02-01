#pragma once
#include <iostream>


//constants
std::string const CLIENT_ID = "id";
std::string const CLIENT_SECRET = "secret";
std::string const redirect_uri = "http://127.0.0.1:8080/callback";

//look at spotifyFUNCS.cpp for more info

std::string randomSTRING(int len);

void svrStarter(std::string state, std::string& auth_code);

std::string authorizationLink(std::string& new_state, std::string& new_uri);

std::string spotifyTOKEN(std::string auth_code, std::string body);

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
