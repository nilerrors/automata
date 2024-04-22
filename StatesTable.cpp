//
// Created by nilerrors on 4/8/24.
//

#include <sstream>
#include "StatesTable.h"
#include "FA.h"
#include "DFA.h"

StatesTable::StatesTable() = default;

StatesTable::~StatesTable() = default;

void StatesTable::from(FA *fa)
{
    std::vector<State *> fa_states = fa->getStates();

    if (fa_states.size() <= 1)
    {
        return;
    }

    std::sort(fa_states.begin(), fa_states.end(), [](State *a, State *b) {
        return a->name > b->name;
    });

    for (State *state: fa_states)
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

    for (State *row: rows)
    {
        for (State *col: cols)
        {
            if (row == col
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

std::string StatesTable::to_string() const
{
    std::stringstream result;

    State *row = nullptr;
    State *col = nullptr;
    for (StateEquivalence eqv: table)
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
        col = eqv.second;

        if (eqv.is_equivalent)
        {
            result << "\t" << "X";
        }
        else
        {
            result << "\t" << "-";
        }
    }

    result << std::endl;
    for (State *col: cols)
    {
        result << "\t" << col->name;
    }

    return result.str();
}
