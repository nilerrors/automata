//
// Created by nilerrors on 2/24/24.
//

#ifndef AUTOMATA_DFA_H
#define AUTOMATA_DFA_H

#include <string>
#include "json.hpp"

#include "FA.h"


class DFA : public FA
{
public:
	DFA();
	[[maybe_unused]]
	explicit DFA(const std::string &file_path);
	explicit DFA(const DFA &first, const DFA &second, bool isIntersection);
	virtual ~DFA();

	[[nodiscard]]
	bool accepts(const std::string& string) const override;
};


#endif //AUTOMATA_DFA_H
