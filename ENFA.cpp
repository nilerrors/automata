//
// Created by nilerrors on 3/12/24.
//

#include "ENFA.h"

ENFA::ENFA()
{
	type = "ENFA";
	allowEpsilonTransitions = true;
}

ENFA::ENFA(const std::string &file_path)
{
	type = "ENFA";
	allowEpsilonTransitions = true;
	fromPath(file_path);
}

ENFA::~ENFA()
{
	clear();
}
