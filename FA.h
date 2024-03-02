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

	Transition(State *from, State *to, std::string_view symbol)
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