//
// Created by nilerrors on 2/24/24.
//

#ifndef AUTOMATA_DFA_H
#define AUTOMATA_DFA_H

#include <string>
#include "json.hpp"

#include "FA.h"
#include "StatesTable.h"


class DFA : public FA
{
public:
    DFA();

    [[maybe_unused]]
    explicit DFA(const std::string &file_path);

    explicit DFA(const DFA &first, const DFA &second, bool isIntersection);

    ~DFA() override;

    [[nodiscard]]
    bool accepts(const std::string &string) const override;

    [[nodiscard]]
    std::shared_ptr<Transition>
    getTransitionFromStateBySymbol(const std::shared_ptr<State> &state, Symbol symbol) const;

    [[nodiscard]]
    DFA minimize() const;

    void printTable() const;

private:
    bool minimized = false;
    std::shared_ptr<StatesTable> table = nullptr;
};


#endif //AUTOMATA_DFA_H
