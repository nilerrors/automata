//
// Created by nilerrors on 4/8/24.
//

#ifndef AUTOMATA_STATESTABLE_H
#define AUTOMATA_STATESTABLE_H

#include <vector>
#include <string>
#include <set>
#include <memory>

class State;

class DFA;

struct StateEquivalence
{
    std::shared_ptr<State> first;
    std::shared_ptr<State> second;
    bool is_distinguishable;

    StateEquivalence()
    {
        first = nullptr;
        second = nullptr;
        is_distinguishable = false;
    }

    StateEquivalence(const std::shared_ptr<State> &f, const std::shared_ptr<State> &s, bool eqv)
    {
        first = f;
        second = s;
        is_distinguishable = eqv;
    }
};

class StatesTable
{
public:
    StatesTable();

    virtual ~StatesTable();

    void from(const DFA *dfa);

    void fill();

    [[nodiscard]]
    std::vector<StateEquivalence> get_indistinguishable() const;

    [[nodiscard]]
    std::string to_string() const;

private:
    bool distinguishable(const std::shared_ptr<State> &first, std::shared_ptr<State> &second) const;

private:
    std::vector<StateEquivalence> table;
    std::vector<std::shared_ptr<State>> rows;
    std::vector<std::shared_ptr<State>> cols;
    DFA const *fa = nullptr;
};


#endif //AUTOMATA_STATESTABLE_H
