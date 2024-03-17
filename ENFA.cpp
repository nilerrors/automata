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


ENFA ENFA::join(const ENFA &first, const ENFA &second)
{
	ENFA result;

	if (first.epsilon != second.epsilon)
		return result;

	result.setEpsilon(first.epsilon);
	std::set_union(first.alphabet.cbegin(), first.alphabet.cend(),
				   second.alphabet.cbegin(), second.alphabet.cend(),
				   std::inserter(result.alphabet, result.alphabet.begin()));

	auto *start = new State("start", true, false);
	auto *end = new State("end", false, true);

	result.addState(start);
	result.addState(end);

	for (const State *state : first.states)
	{
		auto *new_state = new State("join1" + state->name, false, false);
		result.addState(new_state);
		if (state->starting)
		{
			result.addTransition(new Transition(start, new_state, result.epsilon));
		}
		if (state->accepting)
		{
			result.addTransition(new Transition(new_state, end, result.epsilon));
		}
	}
	for (const State *state : second.states)
	{
		auto *new_state = new State("join2" + state->name, false, false);
		result.addState(new_state);
		if (state->starting)
		{
			result.addTransition(new Transition(start, new_state, result.epsilon));
		}
		if (state->accepting)
		{
			result.addTransition(new Transition(new_state, end, result.epsilon));
		}
	}

	for (const Transition *transition : first.transitions)
	{
		result.addTransition(new Transition(
			result.getState("join1" + transition->from->name),
			result.getState("join1" + transition->to->name),
			transition->symbol));
	}
	for (const Transition *transition : second.transitions)
	{
		result.addTransition(new Transition(
			result.getState("join2" + transition->from->name),
			result.getState("join2" + transition->to->name),
			transition->symbol));
	}

	return result;
}

ENFA ENFA::link(const ENFA &first, const ENFA &second)
{
	ENFA result;

	if (first.epsilon != second.epsilon)
		return result;

	result.setEpsilon(first.epsilon);
	std::set_union(first.alphabet.cbegin(), first.alphabet.cend(),
				   second.alphabet.cbegin(), second.alphabet.cend(),
				   std::inserter(result.alphabet, result.alphabet.begin()));

	auto *start = new State("start", true, false);
	auto *end = new State("end", false, true);

	result.addState(start);
	result.addState(end);

	for (const State *state : first.states)
	{
		auto *new_state = new State("link1" + state->name, false, false);
		result.addState(new_state);
		if (state->starting)
		{
			result.addTransition(new Transition(start, new_state, result.epsilon));
		}
	}
	for (const State *state : second.states)
	{
		auto *new_state = new State("link2" + state->name, false, false);
		result.addState(new_state);
		if (state->accepting)
		{
			result.addTransition(new Transition(new_state, end, result.epsilon));
		}
	}

	for (const Transition *transition : first.transitions)
	{
		result.addTransition(new Transition(
			result.getState("link1" + transition->from->name),
			result.getState("link1" + transition->to->name),
			transition->symbol));
		if (transition->to->accepting)
		{
			result.addTransition(new Transition(
				result.getState("link1" + transition->to->name),
				result.getState("link2" + second.getStartingState()->name),
				result.epsilon));
		}
	}
	for (const Transition *transition : second.transitions)
	{
		result.addTransition(new Transition(
			result.getState("link2" + transition->from->name),
			result.getState("link2" + transition->to->name),
			transition->symbol));
	}

	return result;
}

ENFA ENFA::star(const ENFA &enfa)
{
	ENFA result;
	result.setEpsilon(enfa.epsilon);
	result.setAlphabet(enfa.getAlphabet());

	auto *start = new State("start", true, false);
	auto *end = new State("end", false, true);

	result.addTransition(new Transition(start, end, result.epsilon));

	for (const State *state : enfa.getStates())
	{
		auto *new_state = new State("star" + state->name, false, false);
		result.addState(new_state);
		if (state->starting)
		{
			result.addTransition(new Transition(start, new_state, result.epsilon));
		}
		if (state->accepting)
		{
			result.addTransition(new Transition(new_state, end, result.epsilon));
		}
	}

	for (const State *state : enfa.getStates())
	{
		if (state->accepting)
		{
			result.addTransition(new Transition(
				result.getState("star" + state->name),
				result.getStartingState(),
				result.epsilon));
		}
	}

	for (const Transition *transition : enfa.getTransitions())
	{
		result.addTransition(new Transition(
			result.getState("star" + transition->from->name),
			result.getState("star" + transition->to->name),
			transition->symbol));
	}

	return result;
}
