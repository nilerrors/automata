//
// Created by nilerrors on 3/17/24.
//

#ifndef RE_H
#define RE_H
#include "ENFA.h"
#include "FA.h"


class RE
{
public:
	RE(std::string regex, Symbol epsilon);
	~RE();

	[[nodiscard]] ENFA toENFA() const;

	static bool isValid(const std::string &regex);

	static std::set<Symbol> getAlphabet(const std::string &regex, Symbol epsilon);
	static ENFA epsilonToENFA(Symbol epsilon);
	static ENFA symbolToENFA(Symbol symbol, Symbol epsilon);

private:
	Symbol epsilon;
	std::string regex;
};



#endif //RE_H
