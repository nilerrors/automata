//
// Created by nilerrors on 3/12/24.
//

#ifndef ENFA_H
#define ENFA_H

#include "NFA.h"


class ENFA : public NFA
{
public:
	ENFA();
	explicit ENFA(const std::string &file_path);
	virtual ~ENFA();

	// union
	[[nodiscard]] static ENFA join(const ENFA &first, const ENFA &second);
	// concatenation
	[[nodiscard]] static ENFA link(const ENFA &first, const ENFA &second);
	// kleene star
	[[nodiscard]] static ENFA star(const ENFA &enfa);
};


#endif //ENFA_H
