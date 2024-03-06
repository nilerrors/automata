//
// Created by nilerrors on 3/6/24.
//

#include <set>
#include "NFA.h"

NFA::NFA() : FA("NFA") {}

NFA::NFA(const std::string &file_path) : FA("NFA")
{
	fromPath(file_path);
}

NFA::~NFA() = default;

DFA NFA::toDFA() const
{
	DFA dfa;
	dfa.clear();
	dfa.setAlphabet(alphabet);
	auto *starting = new SetOfStates({ getStartingState() });
	auto *dead_state = new SetOfStates({});
	std::vector<SetOfStates *> unprocessed_states = { starting };
	std::vector<SetOfStates *> all_states = { starting };

	if (states.empty() || alphabet.empty() || transitions.empty())
		return dfa;

	auto in_all_states = [&all_states](SetOfStates *set) -> SetOfStates * {
		for (SetOfStates *s : all_states)
		{
			if (s->states.size() == set->states.size() && s->to_string() == set->to_string())
				return s;
		}
		return nullptr;
	};

	dfa.addState(starting->to_state());

	while (!unprocessed_states.empty())
	{
		SetOfStates *current_state = unprocessed_states.back();
		unprocessed_states.pop_back();

		if (!in_all_states(current_state))
			all_states.push_back(current_state);

		for (Symbol symbol : alphabet)
		{
			SetOfStates *next_state = getNextStates(current_state, symbol);
			if (next_state->states.empty())
				continue;

			if (!in_all_states(next_state))
			{
				unprocessed_states.push_back(next_state);
				all_states.push_back(next_state);
				dfa.addState(next_state->to_state());
			}

			dfa.addTransition(
				new Transition(current_state->to_state(), next_state->to_state(), symbol));
		}
	}

	// Add dead state transitions, to make DFA complete
	for (auto state : dfa.getStates())
	{
		std::set<Symbol> uncovered_symbols(dfa.getAlphabet().cbegin(), dfa.getAlphabet().cend());
		for (auto transition : dfa.getTransitions())
		{
			if (state != transition->from)
				continue;

			uncovered_symbols.erase(transition->symbol);
		}

		if (!uncovered_symbols.empty())
		{
			if (!in_all_states(dead_state))
			{
				dfa.addState(dead_state->to_state());
				for (auto symbol : dfa.getAlphabet())
				{
					dfa.addTransition(
						new Transition(dead_state->to_state(), dead_state->to_state(), symbol));
				}
			}
			for (Symbol symbol : uncovered_symbols)
			{
				dfa.addTransition(
					new Transition(state, dead_state->to_state(), symbol));
			}
		}
	}

	return dfa;
}

