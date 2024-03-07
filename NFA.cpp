//
// Created by nilerrors on 3/6/24.
//

#include <deque>
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

	if (states.empty() || alphabet.empty() || transitions.empty())
		return dfa;


	dfa.clear();
	dfa.setAlphabet(alphabet);
	auto *starting = new SetOfStates({ getStartingState() });
	std::deque<SetOfStates *> unprocessed_states = { starting };
	std::set<SetOfStates *> all_states = { starting };

	auto in_all_states = [&all_states](SetOfStates const *set) -> bool {
		return std::any_of(all_states.cbegin(), all_states.cend(), [set](SetOfStates *s) {
			return s->states.size() == set->states.size() && s->to_string() == set->to_string();
		});
	};

	auto get_from_all_states = [&all_states](const std::string &name) -> SetOfStates * {
		for (SetOfStates *s : all_states)
		{
			if (s->to_string() == name)
				return s;
		}
		return nullptr;
	};

	dfa.addState(starting->to_state());

	while (!unprocessed_states.empty())
	{
		SetOfStates *current_state = unprocessed_states.front();
		unprocessed_states.pop_front();

		if (!in_all_states(current_state))
			all_states.insert(current_state);

		for (const Symbol symbol : alphabet)
		{
			SetOfStates *next_state = getNextStates(current_state, symbol);

			if (!in_all_states(next_state))
			{
				unprocessed_states.push_back(next_state);
				all_states.insert(next_state);
				dfa.addState(next_state->to_state());
			}
			else
			{
				std::string name = next_state->to_string();
				delete next_state;
				next_state = get_from_all_states(name);
			}

			dfa.addTransition(
				new Transition(current_state->to_state(), next_state->to_state(), symbol));
		}
	}

	// Remove all temporary set of states
	for (auto &state : all_states)
	{
		delete state;
	}

	return dfa;
}

