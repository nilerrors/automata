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
    std::vector<std::shared_ptr<Transition>> transitionsFromStarting = getTransitionsFromState(getStartingState());
    if (transitionsFromStarting.size() == 1 && transitionsFromStarting.front()->symbol == epsilon)
    {
        std::shared_ptr<State> newStarting = transitionsFromStarting.front()->to;
        newStarting->starting = true;
        for (std::shared_ptr<Transition> &transition: getTransitionsToState(getStartingState()))
        {
            transition->to = newStarting;
        }
        transitions.erase(std::remove(
                transitions.begin(),
                transitions.end(),
                transitionsFromStarting.front()), transitions.end());
        states.erase(std::remove(states.begin(), states.end(), getStartingState()), states.end());
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
            std::shared_ptr<State> newAccept = transitionsToAccepting.front()->from;
            newAccept->accepting = true;
            for (std::shared_ptr<Transition> &transition: getTransitionsFromState(acceptingState))
            {
                transition->from = newAccept;
            }
            transitions.erase(std::remove(
                    transitions.begin(),
                    transitions.end(),
                    transitionsToAccepting.front()), transitions.end());
            states.erase(std::remove(states.begin(), states.end(), acceptingState), states.end());
        }
    }
}


void ENFA::join(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &first, const std::shared_ptr<ENFA> &second)
{
    if (first->epsilon != second->epsilon)
    {
        return;
    }

    result->setEpsilon(first->epsilon);
    std::set_union(first->alphabet.cbegin(), first->alphabet.cend(),
                   second->alphabet.cbegin(), second->alphabet.cend(),
                   std::inserter(result->alphabet, result->alphabet.begin()));

    std::shared_ptr<State> start = std::make_shared<State>("start", true, false);
    std::shared_ptr<State> end = std::make_shared<State>("end", false, true);

    result->addState(start);
    result->addState(end);

    for (const std::shared_ptr<State> &state: first->states)
    {
        std::shared_ptr<State> new_state = std::make_shared<State>("j1" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(std::make_shared<Transition>(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(std::make_shared<Transition>(new_state, end, result->epsilon));
        }
    }
    for (const std::shared_ptr<State> &state: second->states)
    {
        std::shared_ptr<State> new_state = std::make_shared<State>("j2" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(std::make_shared<Transition>(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(std::make_shared<Transition>(new_state, end, result->epsilon));
        }
    }

    for (const std::shared_ptr<Transition> &transition: first->transitions)
    {
        result->addTransition(std::make_shared<Transition>(
                result->getState("j1" + transition->from->name),
                result->getState("j1" + transition->to->name),
                transition->symbol));
    }
    for (const std::shared_ptr<Transition> &transition: second->transitions)
    {
        result->addTransition(std::make_shared<Transition>(
                result->getState("j2" + transition->from->name),
                result->getState("j2" + transition->to->name),
                transition->symbol));
    }
}

void ENFA::link(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &first, const std::shared_ptr<ENFA> &second)
{
    if (first->epsilon != second->epsilon)
    {
        return;
    }

    result->setEpsilon(first->epsilon);
    std::set_union(first->alphabet.cbegin(), first->alphabet.cend(),
                   second->alphabet.cbegin(), second->alphabet.cend(),
                   std::inserter(result->alphabet, result->alphabet.begin()));

    std::shared_ptr<State> end = std::make_shared<State>("end", false, true);

    result->addState(end);

    for (const std::shared_ptr<State> &state: first->states)
    {
        if (state == nullptr)
        {
            continue;
        }
        std::shared_ptr<State> new_state = std::make_shared<State>("l1" + state->name, state->starting, false);
        result->addState(new_state);
    }
    for (const std::shared_ptr<State> &state: second->states)
    {
        if (state == nullptr)
        {
            continue;
        }
        std::shared_ptr<State> new_state = std::make_shared<State>("l2" + state->name, false, false);
        result->addState(new_state);
        if (state->accepting)
        {
            result->addTransition(std::make_shared<Transition>(new_state, end, result->epsilon));
        }
    }

    for (const std::shared_ptr<Transition> &transition: first->transitions)
    {
        result->addTransition(std::make_shared<Transition>(
                result->getState("l1" + transition->from->name),
                result->getState("l1" + transition->to->name),
                transition->symbol));
        if (transition->to->accepting)
        {
            result->addTransition(std::make_shared<Transition>(
                    result->getState("l1" + transition->to->name),
                    result->getState("l2" + second->getStartingState()->name),
                    result->epsilon));
        }
    }
    for (const std::shared_ptr<Transition> &transition: second->transitions)
    {
        result->addTransition(std::make_shared<Transition>(
                result->getState("l2" + transition->from->name),
                result->getState("l2" + transition->to->name),
                transition->symbol));
    }
}

void ENFA::star(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &enfa)
{
    result->setEpsilon(enfa->epsilon);
    result->setAlphabet(enfa->getAlphabet());

    std::shared_ptr<State> start = std::make_shared<State>("start", true, false);
    std::shared_ptr<State> end = std::make_shared<State>("end", false, true);

    result->addState(start);
    result->addState(end);

    result->addTransition(std::make_shared<Transition>(start, end, result->epsilon));

    for (const std::shared_ptr<State> &state: enfa->getStates())
    {
        std::shared_ptr<State> new_state = std::make_shared<State>("s" + state->name, false, false);
        result->addState(new_state);
        if (state->starting)
        {
            result->addTransition(std::make_shared<Transition>(start, new_state, result->epsilon));
        }
        if (state->accepting)
        {
            result->addTransition(std::make_shared<Transition>(new_state, end, result->epsilon));
        }
    }

    for (const std::shared_ptr<State> &state: enfa->getStates())
    {
        if (state->accepting)
        {
            result->addTransition(std::make_shared<Transition>(
                    result->getState("s" + state->name),
                    result->getStartingState(),
                    result->epsilon));
        }
    }

    for (const std::shared_ptr<Transition> &transition: enfa->getTransitions())
    {
        result->addTransition(std::make_shared<Transition>(
                result->getState("s" + transition->from->name),
                result->getState("s" + transition->to->name),
                transition->symbol));
    }
}
