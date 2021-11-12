#include "leaderboard.h"
#include "arguments.h"

#include <sstream>

Leaderboard::Leaderboard() {

}

Leaderboard::~Leaderboard() {

}

bool Leaderboard::Initialize(const Arguments& args, const string &filename) {
    std::cout << "Loading leaderleaderstats from " << filename << std::endl;
    //std::string leaderboardDirectory = args.getResourcePath() + "models/";
    filepath = filename;
    vector<string> tokens;

    // Load data from file
    ifstream file(filename.c_str());
    if (file.good()) { 
        string line;
        if (file.is_open()) {
            while(getline(file, line)) {
                istringstream ssLine(line);
                string token;
                while(getline(ssLine, token, ',')) {
                    tokens.push_back(token);
                }
            }
            file.close();
        }
    }

    for(int i = 0; i < tokens.size(); i+=2) {
        std::cout << tokens[i] << ", " << tokens[i+1] << endl;
        UpdateScore(tokens[i], stof(tokens[i+1]));
    }
    return true;
}

bool Leaderboard::AddPlayer(const string& userId) {
    // Check if player already exists
    if (leaderstats.find(userId) != leaderstats.end())
        return false; 
    
    // Add player to dictionary
    leaderstats.insert(pair<string, float>(userId, 0));
    return true;
}

void Leaderboard::UpdateScore(const string& userId, float score) {
    // Update the players score if its a valid name
    if (userId != "") {
        if (leaderstats.find(userId) == leaderstats.end())
            leaderstats.insert(pair<string, float>(userId, score));
        else
            leaderstats[userId] = score;
    }
}

void Leaderboard::GetTop10Scores() {

}

void Leaderboard::Save() {
    ofstream file(filepath);
    for (auto const& element : leaderstats) {
        file << element.first << "," << to_string(element.second) << endl;
    }
    //file << "write this to file";
    file.close();
}
