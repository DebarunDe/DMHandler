#include <curl/curl.h>
#include <iostream>
#include <regex>
#include <cmath>


size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string httpGet(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
    }
    return response;
}

bool approximatelyEqual(double a, double b, double epsilon = 1e-6) {
    return abs(a - b) < epsilon;
}

bool fieldMatches(const std::string& response, const std::string& key, double expected) {
    std::regex pattern("\"" + key + R"(\":\s*([0-9.]+))");
    std::smatch match;
    if (std::regex_search(response, match, pattern)) {
        double actual = std::stod(match[1]);
        return approximatelyEqual(actual, expected);
    }
    return false;
}