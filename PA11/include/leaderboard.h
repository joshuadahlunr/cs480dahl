#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <map>
#include <string>
#include <iostream>
#include <fstream>


class Arguments;

// Class holding the Leaderboard
class Leaderboard {
public:
	Leaderboard() {}
	~Leaderboard() {}
	bool initialize(const Arguments& args, const std::string &filename);
	bool addPlayer(const std::string& userId);
	void updateScore(const std::string& userId, float score);
	void save();

	std::map<std::string, float> leaderstats;

private:

	void sort();
	std::string filepath;
};

#endif /* Leaderboard_H */
