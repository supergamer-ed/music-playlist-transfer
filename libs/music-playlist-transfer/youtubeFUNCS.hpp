#pragma once
#include <string>

namespace YT{
    const std::string CLIENT_ID = "ytid";
    const std::string REDIRECT_URI = "http://127.0.0.1:8080/callback";
    const std::string CLIENT_SECRET = "ytsecret";
    void svrStarter(std::string state, std::string& auth_code);
};

std::string ytauthlink(std::string& state);
