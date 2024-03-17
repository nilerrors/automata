//
// Created by nilerrors on 3/6/24.
//

#ifndef NFA_H
#define NFA_H

#include "FA.h"
#include "DFA.h"


class NFA : public FA
{
public:
	NFA();
	[[maybe_unused]]
	explicit NFA(const std::string &file_path);
	virtual ~NFA();

	[[nodiscard]]
	DFA toDFA() const;

	[[nodiscard]]
	bool accepts(const std::string &string) const override;
};



#endif //NFA_H
