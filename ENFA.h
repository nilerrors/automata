//
// Created by nilerrors on 3/12/24.
//

#ifndef ENFA_H
#define ENFA_H

#include "FA.h"
#include "NFA.h"


class ENFA : public NFA
{
public:
	ENFA();
	explicit ENFA(const std::string &file_path);
	virtual ~ENFA();
};


#endif //ENFA_H
