//
// Created by nilerrors on 2/24/24.
//

#include <fstream>
#include <iomanip>
#include <deque>
#include <iostream>

#include "DFA.h"


DFA::DFA() : FA("DFA")
{
    json j;
    j["type"] = "DFA";
    j["alphabet"] = {"0", "1"};
    j["states"] = {
            {{"name", "s0"}, {"starting", true},  {"accepting", true}},
            {{"name", "s1"}, {"starting", false}, {"accepting", false}},
            {{"name", "s2"}, {"starting", false}, {"accepting", false}}
    };
    j["transitions"] = {
            {{"from", "s0"}, {"to", "s0"}, {"input", "0"}},
            {{"from", "s0"}, {"to", "s1"}, {"input", "1"}},
            {{"from", "s1"}, {"to", "s1"}, {"input", "0"}},
            {{"from", "s1"}, {"to", "s0"}, {"input", "1"}},
            {{"from", "s2"}, {"to", "s1"}, {"input", "0"}},
            {{"from", "s2"}, {"to", "s2"}, {"input", "1"}}
    };

    fromJSON(j);
}

[[maybe_unused]]
DFA::DFA(const std::string &file_path) : FA("DFA")
{
    fromPath(file_path);
}

DFA::DFA(const DFA &first, const DFA &second, const bool isIntersection) : FA("DFA")
{
    // Product construction for intersection and union
    clear();

    std::set_intersection(
            first.getAlphabet().cbegin(), first.getAlphabet().cend(),
            second.getAlphabet().cbegin(), second.getAlphabet().cend(),
            std::inserter(alphabet, alphabet.begin()));

    auto isStarting = [](const std::shared_ptr<State> &s1, const std::shared_ptr<State> &s2) -> bool {
        return s1->starting && s2->starting;
    };

    auto isAccepting = [&isIntersection](const std::shared_ptr<State> &s1, const std::shared_ptr<State> &s2) -> bool {
        if (!isIntersection)
        {
            return s1->accepting || s2->accepting;
        }
        return s1->accepting && s2->accepting;
    };

    std::shared_ptr<PairOfStates> startingState =
            std::make_shared<PairOfStates>(first.getStartingState(), second.getStartingState(), true,
                                           isAccepting(first.getStartingState(), second.getStartingState()));

    std::deque<std::shared_ptr<PairOfStates>> unprocessed_states = {startingState};
    std::set<std::shared_ptr<PairOfStates>> processed_states = {startingState};
    std::set<std::shared_ptr<PairOfStates>> all_pairs_of_states = {startingState};


    auto in_processed_states = [&processed_states](const std::shared_ptr<PairOfStates> &pair) -> bool {
        for (const std::shared_ptr<PairOfStates> &state: processed_states)
        {
            return (state->states.first == pair->states.first &&
                    state->states.second == pair->states.second)
                   || (state->states.first == pair->states.second &&
                       state->states.second == pair->states.first);
        }
        return false;
    };

    auto from_all_states = [&all_pairs_of_states](const std::shared_ptr<State> &f,
                                                  const std::shared_ptr<State> &s) -> std::shared_ptr<PairOfStates> {
        for (const std::shared_ptr<PairOfStates> &state: all_pairs_of_states)
        {
            if ((state->states.first == f && state->states.second == s)
                || (state->states.first == s && state->states.second == f))
            {
                return state;
            }
        }
        return nullptr;
    };

    while (!unprocessed_states.empty())
    {
        std::shared_ptr<PairOfStates> state = unprocessed_states.front();
        unprocessed_states.pop_front();

        all_pairs_of_states.insert(state);
        addState(state->to_state());

        for (const Symbol symbol: alphabet)
        {
            if (first.getNextState(state->states.first, symbol) == state->states.first &&
                second.getNextState(state->states.second, symbol) == state->states.second)
            {
                addTransition(std::make_shared<Transition>(state->to_state(), state->to_state(), symbol));
                continue;
            }

            std::shared_ptr<PairOfStates> to = from_all_states(
                    first.getNextState(state->states.first, symbol),
                    second.getNextState(state->states.second, symbol));

            if (to == nullptr)
            {
                to = std::make_shared<PairOfStates>(
                        first.getNextState(state->states.first, symbol),
                        second.getNextState(state->states.second, symbol),
                        isStarting(first.getNextState(state->states.first, symbol),
                                   second.getNextState(state->states.second, symbol)),
                        isAccepting(first.getNextState(state->states.first, symbol),
                                    second.getNextState(state->states.second, symbol)));
            }

            addState(to->to_state());

            if (!in_processed_states(to))
            {
                unprocessed_states.push_back(to);
            }

            addTransition(
                    std::make_shared<Transition>(
                            state->to_state(),
                            to->to_state(), symbol));
        }
        processed_states.insert(state);
    }
}

DFA::~DFA() = default;

bool DFA::accepts(const std::string &string) const
{
    std::shared_ptr<State> currentState = getStartingState();
    if (currentState == nullptr)
    {
        return false;
    }

    for (const Symbol c: string)
    {
        currentState = getNextState(currentState, c);
        if (currentState == nullptr)
        {
            return false;
        }
    }
    return currentState->accepting;
}

DFA DFA::minimize() const
{
    DFA min;
    min.clear();
    min.minimized = true;
    min.table = std::make_shared<StatesTable>();
    min.table->from(this);
    min.table->fill();

    std::vector<std::shared_ptr<SetOfStates>> merged_states;

    auto in_merged_states = [&](const std::shared_ptr<State> &state) -> std::pair<bool, std::shared_ptr<SetOfStates>> {
        for (const std::shared_ptr<SetOfStates> &set_of_states: merged_states)
        {
            auto found = set_of_states->states.find(state);
            if (found != set_of_states->states.end())
            {
                for (const std::shared_ptr<State> &s: set_of_states->states)
                {
                    if (s == state)
                    {
                        return std::make_pair(true, set_of_states);
                    }
                }
            }
        }
        return std::make_pair(false, nullptr);
    };

    for (StateEquivalence eqv: min.table->get_indistinguishable())
    {
        std::pair<bool, std::shared_ptr<SetOfStates>> first_in_merged = in_merged_states(eqv.first);
        std::pair<bool, std::shared_ptr<SetOfStates>> second_in_merged = in_merged_states(eqv.second);

        if (!first_in_merged.first && !second_in_merged.first)
        {
            merged_states.push_back(std::make_shared<SetOfStates>(std::set({eqv.first, eqv.second})));
        }
        else if (first_in_merged.first)
        {
            first_in_merged.second->add(eqv.second);
        }
        else if (second_in_merged.second)
        {
            second_in_merged.second->add(eqv.first);
        }
    }

    for (const std::shared_ptr<State> &state: states)
    {
        std::pair<bool, std::shared_ptr<SetOfStates>> state_in_merged = in_merged_states(state);
        if (!state_in_merged.first)
        {
            merged_states.push_back(std::make_shared<SetOfStates>(std::set({state})));
        }
    }

    for (const std::shared_ptr<SetOfStates> &ms: merged_states)
    {
        min.addState(ms->to_state());
    }

//    for (const SetOfStates &ms: merged_states)
//    {
//        std::cout << ms.to_string() << std::endl;
//    }
//
    return min;
}

void DFA::printTable() const
{
    if (!minimized || table == nullptr)
    {
        return;
    }

    std::cout << table->to_string() << std::endl;
}

std::shared_ptr<Transition>
DFA::getTransitionFromStateBySymbol(const std::shared_ptr<State> &state, Symbol symbol) const
{
    for (const std::shared_ptr<Transition> &transition: getTransitionsFromState(state))
    {
        if (transition->symbol == symbol)
        {
            return transition;
        }
    }

    return nullptr;
}
