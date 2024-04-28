//
// Created by nilerrors on 3/6/24.
//

#include <deque>
#include "NFA.h"

NFA::NFA() : FA("NFA")
{
}

NFA::NFA(const std::string &file_path) : FA("NFA")
{
    fromPath(file_path);
}

NFA::~NFA() = default;

bool NFA::accepts(const std::string &string) const
{
    return toDFA().accepts(string);
}

DFA NFA::toDFA() const
{
    DFA dfa;

    if (states.empty() || alphabet.empty() || transitions.empty())
    {
        return dfa;
    }


    dfa.clear();
    dfa.setAlphabet(alphabet);
    std::shared_ptr<SetOfStates> starting = getStartingStates();
    std::deque<std::shared_ptr<SetOfStates>> unprocessed_states = {starting};
    std::set<std::shared_ptr<SetOfStates>> all_states = {starting};

    auto in_all_states = [&all_states](const std::shared_ptr<SetOfStates> &set) -> bool {
        return std::any_of(all_states.cbegin(), all_states.cend(), [set](const std::shared_ptr<SetOfStates> &s) {
            return s->states.size() == set->states.size() && s->to_string() == set->to_string();
        });
    };

    auto get_from_all_states = [&all_states](const std::string &name) -> std::shared_ptr<SetOfStates> {
        for (const std::shared_ptr<SetOfStates> &s: all_states)
        {
            if (s->to_string() == name)
            {
                return s;
            }
        }
        return nullptr;
    };

    dfa.addState(starting->to_state());

    while (!unprocessed_states.empty())
    {
        std::shared_ptr<SetOfStates> current_state = unprocessed_states.front();
        unprocessed_states.pop_front();

        if (!in_all_states(current_state))
        {
            all_states.insert(current_state);
        }

        for (const Symbol symbol: alphabet)
        {
            std::shared_ptr<SetOfStates> next_state = getNextStates(current_state, symbol);

            if (!in_all_states(next_state))
            {
                unprocessed_states.push_back(next_state);
                all_states.insert(next_state);
                dfa.addState(next_state->to_state());
            }
            else
            {
                std::string name = next_state->to_string();
                next_state = get_from_all_states(name);
            }

            dfa.addTransition(std::make_shared<Transition>(current_state->to_state(), next_state->to_state(), symbol));
        }
    }

    return dfa;
}
