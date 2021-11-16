#include "leaderboard.h"
#include "arguments.h"

#include <vector>

#include <sstream>

bool Leaderboard::initialize(const Arguments& args, const std::string &filename) {
    //std::cout << "Loading leaderleaderstats from " << filename << std::endl;
    //std::string leaderboardDirectory = args.getResourcePath() + "models/";
    filepath = filename;
    std::vector<std::string> tokens;

    // Load data from file
    std::ifstream file(filename.c_str());
    if (file) {
        std::string line;
        if (file.is_open()) {
            while(getline(file, line)) {
                std::istringstream ssLine(line);
                std::string token;
                while(getline(ssLine, token, ',')) {
                    tokens.push_back(token);
                }
            }
            file.close();
        }
    }

    for(int i = 0; i < tokens.size(); i+=2) {
        std::cout << tokens[i] << ", " << tokens[i+1] << std::endl;
        updateScore(tokens[i], stof(tokens[i+1]));
    }
    return true;
}

bool Leaderboard::addPlayer(const std::string& userId) {
    // Check if player already exists
    if (leaderstats.find(userId) != leaderstats.end())
        return false;

    // Add player to dictionary
    leaderstats.insert(std::pair<std::string, float>(userId, 0));
    return true;
}

void Leaderboard::updateScore(const std::string& userId, float score) {
    // Update the players score if its a valid name
    if (userId != "") {
        if (leaderstats.find(userId) == leaderstats.end())
            leaderstats.insert(std::pair<std::string, float>(userId, score));
        else
            leaderstats[userId] = score;
    }
    sort();
}

void Leaderboard::save() {
    std::ofstream file(filepath);
    for (auto const& element : leaderstats) {
        file << element.first << "," << std::to_string(element.second) << std::endl;
    }
    //file << "write this to file";
    file.close();
}

bool compare(std::pair<std::string, float>& a, std::pair<std::string, float>& b) {
    return a.second < b.second;
}

void Leaderboard::sort() {
    // Sort the leaderboard
    std::vector<std::pair<std::string, float>> items;
    for (auto& it : leaderstats) {
        items.push_back(it);
    }

    std::sort(items.begin(), items.end(), compare);

    leaderstats.clear();
    for (auto& it : items) {
        leaderstats.insert(it);
    }
}
