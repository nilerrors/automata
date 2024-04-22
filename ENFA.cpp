//
// Created by nilerrors on 3/12/24.
//

#include "ENFA.h"

ENFA::ENFA()
{
    type = "ENFA";
    allowEpsilonTransitions = true;
}

ENFA::ENFA(const std::string &file_path)
{
    type = "ENFA";
    allowEpsilonTransitions = true;
    fromPath(file_path);
}

ENFA::~ENFA()
{
    clear();
}

void ENFA::optimizeStart()
{
    auto transitionsFromStarting = getTransitionsFromState(getStartingState());
    if (transitionsFromStarting.size() == 1 && transitionsFromStarting.front()->symbol == epsilon)
    {
        auto *newStarting = transitionsFromStarting.front()->to;
        newStarting->starting = true;
        for (auto &transition: getTransitionsToState(getStartingState()))
        {
            transition->to = newStarting;
        }
        transitions.erase(std::remove(
                transitions.begin(),
                transitions.end(),
                transitionsFromStarting.front()), transitions.end());
        states.erase(std::remove(states.begin(), states.end(), getStartingState()), states.end());
        delete getStartingState();
        startingState = newStarting;
    }

}

void ENFA::optimizeAccept()
{
    if (getAcceptingStates().size() == 1)
    {
        auto acceptingState = getAcceptingStates().front();
        auto transitionsToAccepting = getTransitionsToState(acceptingState);
        if (transitionsToAccepting.size() == 1 && transitionsToAccepting.front()->symbol == epsilon)
        {
            auto *newAccept = transitionsToAccepting.front()->from;
            newAccept->accepting = true;
            for (auto &transition: getTransitionsFromState(acceptingState))
            {
                transition->from = newAccept;
            }
            transitions.erase(std::remove(
                    transitions.begin(),
                    transitions.end(),
                    transitionsToAccepting.front()), transitions.end());
            states.erase(std::remove(states.begin(), states.end(), acceptingState), states.end());
            delete acceptingState;
        }
    }
}


void ENFA::join(ENFA *result, ENFA const *first, ENFA const *second)
{
    if (first->epsilon != second->epsilon)
    {
        return;
    }

    result->setEpsilon(first->epsilon);
    std::set_union(first->alphabet.cbegin(), first->alphabet.cend(),
                   second->alphabet.cbegin(), second->alphabet.cend(),
                   std::inserter(result->alphabet, result->alphabet.begin()));

    auto *start = new State("start", true, false);
    auto *end = new State("end", false, true);

    result->addState(start);
    result->addState(end);

    for (const State *state: first->states)
    {
        auto *new_state = new State("j1" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(new Transition(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(new Transition(new_state, end, result->epsilon));
        }
    }
    for (const State *state: second->states)
    {
        auto *new_state = new State("j2" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(new Transition(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(new Transition(new_state, end, result->epsilon));
        }
    }

    for (const Transition *transition: first->transitions)
    {
        result->addTransition(new Transition(
                result->getState("j1" + transition->from->name),
                result->getState("j1" + transition->to->name),
                transition->symbol));
    }
    for (const Transition *transition: second->transitions)
    {
        result->addTransition(new Transition(
                result->getState("j2" + transition->from->name),
                result->getState("j2" + transition->to->name),
                transition->symbol));
    }
}

void ENFA::link(ENFA *result, ENFA const *first, ENFA const *second)
{
    if (first->epsilon != second->epsilon)
    {
        return;
    }

    result->setEpsilon(first->epsilon);
    std::set_union(first->alphabet.cbegin(), first->alphabet.cend(),
                   second->alphabet.cbegin(), second->alphabet.cend(),
                   std::inserter(result->alphabet, result->alphabet.begin()));

    auto *end = new State("end", false, true);

    result->addState(end);

    for (const State *state: first->states)
    {
        if (state == nullptr)
        {
            continue;
        }
        auto *new_state = new State("l1" + state->name, state->starting, false);
        result->addState(new_state);
    }
    for (const State *state: second->states)
    {
        if (state == nullptr)
        {
            continue;
        }
        auto *new_state = new State("l2" + state->name, false, false);
        result->addState(new_state);
        if (state->accepting)
        {
            result->addTransition(new Transition(new_state, end, result->epsilon));
        }
    }

    for (const Transition *transition: first->transitions)
    {
        result->addTransition(new Transition(
                result->getState("l1" + transition->from->name),
                result->getState("l1" + transition->to->name),
                transition->symbol));
        if (transition->to->accepting)
        {
            result->addTransition(new Transition(
                    result->getState("l1" + transition->to->name),
                    result->getState("l2" + second->getStartingState()->name),
                    result->epsilon));
        }
    }
    for (const Transition *transition: second->transitions)
    {
        result->addTransition(new Transition(
                result->getState("l2" + transition->from->name),
                result->getState("l2" + transition->to->name),
                transition->symbol));
    }
}

void ENFA::star(ENFA *result, ENFA const *enfa)
{
    result->setEpsilon(enfa->epsilon);
    result->setAlphabet(enfa->getAlphabet());

    auto *start = new State("start", true, false);
    auto *end = new State("end", false, true);

    result->addState(start);
    result->addState(end);

    result->addTransition(new Transition(start, end, result->epsilon));

    for (const State *state: enfa->getStates())
    {
        auto *new_state = new State("s" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(new Transition(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(new Transition(new_state, end, result->epsilon));
        }
    }

    for (const State *state: enfa->getStates())
    {
        if (state->accepting)
        {
            result->addTransition(new Transition(
                    result->getState("s" + state->name),
                    result->getStartingState(),
                    result->epsilon));
        }
    }

    for (const Transition *transition: enfa->getTransitions())
    {
        result->addTransition(new Transition(
                result->getState("s" + transition->from->name),
                result->getState("s" + transition->to->name),
                transition->symbol));
    }
}
