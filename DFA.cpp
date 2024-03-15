//
// Created by nilerrors on 2/24/24.
//

#include <fstream>
#include <iomanip>
#include <deque>

#include "DFA.h"


DFA::DFA() : FA("DFA")
{
	json j;
	j["type"] = "DFA";
	j["alphabet"] = {"0", "1"};
	j["states"] = {
			{{"name", "s0"},{"starting", true},{"accepting", true}},
			{{"name", "s1"},{"starting", false},{"accepting", false}},
			{{"name", "s2"},{"starting", false},{"accepting", false}}
	};
	j["transitions"] = {
			{{"from", "s0"},{"to", "s0"},{"input", "0"}},
			{{"from", "s0"},{"to", "s1"},{"input", "1"}},
			{{"from", "s1"},{"to", "s1"},{"input", "0"}},
			{{"from", "s1"},{"to", "s0"},{"input", "1"}},
			{{"from", "s2"},{"to", "s1"},{"input", "0"}},
			{{"from", "s2"},{"to", "s2"},{"input", "1"}}
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
	// Product construction for intersection
	clear();

	std::set_intersection(
		first.getAlphabet().cbegin(), first.getAlphabet().cend(),
		second.getAlphabet().cbegin(), second.getAlphabet().cend(),
		std::inserter(alphabet, alphabet.begin()));

	auto isStarting = [](const State *s1, const State *s2) -> bool {
		return s1->starting && s2->starting;
	};

	auto isAccepting = [&isIntersection](const State *s1, const State *s2) -> bool {
		if (!isIntersection)
			return s1->accepting || s2->accepting;
		return s1->accepting && s2->accepting;
	};

	auto *startingState = new PairOfStates(
		first.getStartingState(), second.getStartingState(), true,
		isAccepting(first.getStartingState(), second.getStartingState()));

	std::deque<PairOfStates *> unprocessed_states = { startingState };
	std::set<PairOfStates *> processed_states = { startingState };
	std::set<PairOfStates *> all_pairs_of_states = { startingState };

	auto in_processed_states = [&processed_states](const PairOfStates *pair) -> bool {
		return std::any_of(processed_states.begin(), processed_states.end(), [pair](const PairOfStates *state) {
			return (state->states.first == pair->states.first && state->states.second == pair->states.second)
				|| (state->states.first == pair->states.second && state->states.second == pair->states.first);
		});
	};

	auto from_all_states = [&all_pairs_of_states](const State *f, const State *s) -> PairOfStates * {
		for (const auto state : all_pairs_of_states)
		{
			if ((state->states.first == f && state->states.second == s)
				|| (state->states.first == s && state->states.second == f))
				return state;
		}
		return nullptr;
	};

	while (!unprocessed_states.empty())
	{
		auto *state = unprocessed_states.front();
		unprocessed_states.pop_front();

		all_pairs_of_states.insert(state);
		addState(state->to_state());

		for (const Symbol symbol : alphabet)
		{
			if (first.getNextState(state->states.first, symbol) == state->states.first &&
				second.getNextState(state->states.second, symbol) == state->states.second)
			{
				addTransition(new Transition(state->to_state(), state->to_state(), symbol));
				continue;
			}

			auto *to = from_all_states(
				first.getNextState(state->states.first, symbol),
				second.getNextState(state->states.second, symbol));

			if (to == nullptr)
			{
				to = new PairOfStates(
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
				new Transition(
					state->to_state(),
					to->to_state(), symbol));
		}
		processed_states.insert(state);
	}

	for (const PairOfStates *state : all_pairs_of_states)
	{
		delete state;
	}
}

DFA::~DFA() = default;

bool DFA::accepts(const std::string& string) const
{
	const State* currentState = getStartingState();
	if (currentState == nullptr)
		return false;

	for (const Symbol c : string)
	{
		currentState = getNextState(currentState, c);
		if (currentState == nullptr)
			return false;
	}
	return currentState->accepting;
}
