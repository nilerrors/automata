//
// Created by nilerrors on 4/8/24.
//

#include <sstream>
#include "StatesTable.h"
#include "FA.h"
#include "DFA.h"

StatesTable::StatesTable() = default;

StatesTable::~StatesTable() = default;

void StatesTable::from(const DFA *dfa)
{
    fa = dfa;
    std::vector<std::shared_ptr<State>> fa_states = fa->getStates();

    if (fa_states.size() <= 1)
    {
        return;
    }

    std::sort(fa_states.begin(), fa_states.end(), [](const std::shared_ptr<State> &a, const std::shared_ptr<State> &b) {
        return a->name < b->name;
    });

    for (const std::shared_ptr<State> &state: fa_states)
    {
        if (state != fa_states.front())
        {
            rows.push_back(state);
        }
        if (state != fa_states.back())
        {
            cols.push_back(state);
        }
    }

    for (const std::shared_ptr<State> &row: rows)
    {
        for (std::shared_ptr<State> &col: cols)
        {
            if (row == col || row->name < col->name
                || std::any_of(table.begin(), table.end(), [&](StateEquivalence eqv) -> bool {
                return (eqv.first == row && eqv.second == col) || (eqv.first == col && eqv.second == row);
            }))
            {
                continue;
            }
            table.emplace_back(row, col, false);
        }
    }
}

void StatesTable::fill()
{
    // X0   -> final and non-final states are distinguishable
    for (StateEquivalence &eqv: table)
    {
        if ((eqv.first->accepting && !eqv.second->accepting)
            || (!eqv.first->accepting && eqv.second->accepting))
        {
            eqv.is_distinguishable = true;
        }
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (StateEquivalence &eqv: table)
        {
            if (!eqv.is_distinguishable)
            {
                for (Symbol symbol: fa->getAlphabet())
                {
                    std::shared_ptr<State> next_first = fa->getTransitionFromStateBySymbol(eqv.first, symbol)->to;
                    std::shared_ptr<State> next_second = fa->getTransitionFromStateBySymbol(eqv.second, symbol)->to;
                    if (next_first != nullptr && next_second != nullptr
                        && distinguishable(next_first, next_second))
                    {
                        eqv.is_distinguishable = true;
                        changed = true;
                        break;
                    }
                }
            }
        }
    }
}

std::string StatesTable::to_string() const
{
    std::stringstream result;

    std::shared_ptr<State> row = nullptr;
    for (const StateEquivalence &eqv: table)
    {
        if (eqv.first != row)
        {
            if (row != nullptr)
            {
                result << std::endl;
            }
            result << eqv.first->name;
        }

        row = eqv.first;

        if (eqv.is_distinguishable)
        {
            result << "\t" << "X";
        }
        else
        {
            result << "\t" << "-";
        }
    }

    result << std::endl;
    for (const std::shared_ptr<State> &col: cols)
    {
        result << "\t" << col->name;
    }

    return result.str();
}

bool StatesTable::distinguishable(const std::shared_ptr<State> &first, std::shared_ptr<State> &second) const
{
    for (const StateEquivalence &eqv: table)
    {
        if ((eqv.first == first && eqv.second == second))
        {
            return eqv.is_distinguishable;
        }
    }
    return false;
}

std::vector<StateEquivalence> StatesTable::get_indistinguishable() const
{
    std::vector<StateEquivalence> indistinguishable;

    for (const StateEquivalence &eqv: table)
    {
        if (!eqv.is_distinguishable)
        {
            indistinguishable.push_back(eqv);
        }
    }

    return indistinguishable;
}
