//
// Created by nilerrors on 4/8/24.
//

#ifndef AUTOMATA_STATESTABLE_H
#define AUTOMATA_STATESTABLE_H

#include <vector>
#include <string>

class State;

class FA;

struct StateEquivalence
{
    State *first;
    State *second;
    bool is_equivalent;

    StateEquivalence()
    {
        first = nullptr;
        second = nullptr;
        is_equivalent = false;
    }

    StateEquivalence(State *f, State *s, bool eqv)
    {
        first = f;
        second = s;
        is_equivalent = eqv;
    }
};

class StatesTable
{
public:
    StatesTable();

    virtual ~StatesTable();

    void from(FA *fa);

    [[nodiscard]]
    std::string to_string() const;

private:
    std::vector<StateEquivalence> table;
    std::vector<State *> rows;
    std::vector<State *> cols;
};


#endif //AUTOMATA_STATESTABLE_H
