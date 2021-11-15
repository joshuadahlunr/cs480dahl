#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <map>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

class Arguments;

// Class holding the Leaderboard
class Leaderboard {
public:
	Leaderboard();
	~Leaderboard();
	bool Initialize(const Arguments& args, const string &filename);
	bool AddPlayer(const string& userId);
	void UpdateScore(const string& userId, float score);
	void Save();

	map<string, float> leaderstats;

private:

	void sort();
	string filepath;
};

#endif /* Leaderboard_H */
