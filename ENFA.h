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

	void optimizeStart();
	void optimizeAccept();

	// union
	static void join(ENFA *into, ENFA const *first, ENFA const *second);
	// concatenation
	static void link(ENFA *into, ENFA const *first, ENFA const *second);
	// kleene star
	static void star(ENFA *into, ENFA const *enfa);
};


#endif //ENFA_H
