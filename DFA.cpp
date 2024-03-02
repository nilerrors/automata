//
// Created by nilerrors on 2/24/24.
//

#include <fstream>
#include <iostream>
#include <iomanip>

#include "DFA.h"


DFA::DFA() : FA("DFA")
{
	json j;
	j["type"] = "DFA";
	j["alphabet"] = {"0", "1"};
	j["states"] = {
			{{"name", "s0"},{"starting", true},{"accepting", true}},
			{{"name", "s1"},{"starting", false},{"accepting", false}},
			{{"name", "s2"},{"starting", false},{"accepting", false}}
	};
	j["transitions"] = {
			{{"from", "s0"},{"to", "s0"},{"input", "0"}},
			{{"from", "s0"},{"to", "s1"},{"input", "1"}},
			{{"from", "s1"},{"to", "s1"},{"input", "0"}},
			{{"from", "s1"},{"to", "s0"},{"input", "1"}},
			{{"from", "s2"},{"to", "s1"},{"input", "0"}},
			{{"from", "s2"},{"to", "s2"},{"input", "1"}}
	};

	fromJSON(j);
}

[[maybe_unused]]
DFA::DFA(const std::string &file_path) : FA("DFA")
{
	FA::type = "DFA";

	fromPath(file_path);
}

DFA::~DFA() = default;

bool DFA::accepts(const std::string& string) const
{
	State* currentState = getStartingState();
	if (currentState == nullptr)
		return false;

	for (char c : string)
	{
		currentState = getNextState(currentState, c);
		if (currentState == nullptr)
			return false;
	}
	return currentState->accepting;
}
