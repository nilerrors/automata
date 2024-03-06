//
// Created by nilerrors on 3/2/24.
//

#ifndef AUTOMATA_FA_H
#define AUTOMATA_FA_H

#include <string>
#include <memory>
#include "json.hpp"

using json = nlohmann::json;
using Symbol = char;

const int SYMBOL_SIZE = 1;

struct State
{
	std::string name;
	bool starting;
	bool accepting;

	State()
	{
		name = "";
		starting = false;
		accepting = false;
	}

	State(const std::string &name, bool isBegin, bool isEnd)
	{
		State::name = name;
		starting = isBegin;
		accepting = isEnd;
	}

	[[nodiscard]]
	json to_json() const
	{
		json j;

		j["name"] = name;
		j["starting"] = starting;
		j["accepting"] = accepting;

		return j;
	}
};

struct SetOfStates
{
	std::vector<State *> states;
	State *state = nullptr;

	explicit SetOfStates(const std::vector<State *> &states) : states(states) {}

	State *to_state()
	{
		if (state != nullptr)
			return state;
		state = new State();
		state->name = to_string();
		state->starting = isStarting();
		state->accepting = isAccepting();
		return state;
	}

	bool isStarting() const
	{
		return states.size() == 1
				&& std::any_of(
					states.begin(),
					states.end(),
					[](const State *state) { return state->starting; });
	}

	bool isAccepting() const
	{
		return std::any_of(
			states.begin(),
			states.end(),
			[](const State *state) { return state->accepting; });
	}

	[[nodiscard]]
	std::string to_string() const
	{
		std::string result = "{";
		for (const auto &state : states)
		{
			if (state != states.front())
				result += ",";
			result += state->name;
		}
		result += "}";
		return result;
	}
};


struct Transition
{
	State *from;
	State *to;
	Symbol symbol;

	Transition(State *from, State *to, Symbol symbol)
	{
		Transition::from = from;
		Transition::to = to;
		Transition::symbol = symbol;
	}

	Transition(State *from, State *to, std::string symbol)
	{
		Transition::from = from;
		Transition::to = to;
		Transition::symbol = symbol.front();
	}

	[[nodiscard]]
	json to_json() const
	{
		json j;

		j["from"] = from->name;
		j["to"] = to->name;
		j["input"] = std::string().assign(1, symbol);

		return j;
	}
};

class FA
{
public:
	explicit FA(const std::string &type);
	virtual ~FA();

	void clear();

	void fromPath(const std::string &file_path);
	void fromJSON(const json &j);

	[[nodiscard]]
	virtual json to_json() const;

	void print() const;

	void addState(State *state) { states.push_back(state); }
	void addTransition(Transition *transition) { transitions.push_back(transition); }

	[[nodiscard]] [[maybe_unused]]
	const std::string &getType() const { return type; }
	[[nodiscard]] [[maybe_unused]]
	const std::vector<Symbol> &getAlphabet() const { return alphabet; }
	[[nodiscard]] [[maybe_unused]]
	State* getStartingState() const { return (startingStates.empty() ? nullptr : startingStates.front()); }
	[[nodiscard]] [[maybe_unused]]
	const std::vector<State *> &getStates() const { return states; }
	[[nodiscard]] [[maybe_unused]]
	const std::vector<Transition *> &getTransitions() const { return transitions; }

	[[nodiscard]]
	State *getState(const std::string &name) const
	{
		for (auto state : states)
		{
			if (state->name == name)
				return state;
		}
		return nullptr;
	}

	[[nodiscard]]
	State *getNextState(State *from, Symbol symbol) const
	{
		for (auto transition : transitions)
		{
			if (transition->from == from && transition->symbol == symbol)
				return transition->to;
		}
		return nullptr;
	}

	[[nodiscard]]
	SetOfStates *getNextStates(const SetOfStates *from, Symbol symbol) const
	{
		auto *nextStates = new SetOfStates({});
		for (auto state : from->states)
		{
			for (auto transition : transitions)
			{
				if (transition->from == state && transition->symbol == symbol)
					nextStates->states.push_back(transition->to);
			}
		}
		return nextStates;
	}

	void setAlphabet(const std::vector<Symbol> &alphabet) { FA::alphabet = alphabet; }

protected:
	void validateAlphabetAndStore(const nlohmann::basic_json<> &alphabet_array);
	void validateStatesAndStore(const nlohmann::basic_json<> &states_array);
	void validateTransitionsAndStore(const nlohmann::basic_json<> &transitions_array);

protected:
	std::string type;
	std::vector<Symbol> alphabet;
	std::vector<State *> states;
	std::vector<Transition *> transitions;
	std::vector<State *> startingStates;

	bool allowMultipleStartStates = false;
};


#endif //AUTOMATA_FA_H
