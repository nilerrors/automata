//
// Created by nilerrors on 2/24/24.
//

#ifndef DFA_DFA_H
#define DFA_DFA_H

#include <vector>
#include <unordered_map>
#include <string>
#include "json.hpp"

using json = nlohmann::json;
using symbol = std::string;

const int SYMBOL_SIZE = 1;


struct DFA_State
{
	std::string name;
	bool starting;
	bool accepting;
	std::unordered_map<symbol, DFA_State*> transitions;

	explicit DFA_State(const std::string &name)
	{
		DFA_State::name = name;
		starting = false;
		accepting = false;
		transitions = {};
	}

	DFA_State(const std::string &name, bool isBegin, bool isEnd)
	{
		DFA_State::name = name;
		starting = isBegin;
		accepting = isEnd;
		transitions = {};
	}

	void addTransition(const symbol& symb, DFA_State* state_pointer)
	{
		transitions[symb] = state_pointer;
	}

	DFA_State* getState(const symbol& symb)
	{
		const auto state = transitions.find(symb);
		if (state == transitions.end())
			return nullptr;
		return state->second;
	}

	json to_json() const
	{
		json j;

		j["name"] = name;
		j["starting"] = starting;
		j["accepting"] = accepting;

		return j;
	}

	json transitions_to_json() const
	{
		json j;

		for (const auto& transition : transitions)
		{
			json transition_json;
			transition_json["from"] = name;
			transition_json["to"] = transition.second->name;
			transition_json["input"] = transition.first;

			j.push_back(transition_json);
		}

		return j;
	}
};

class DFA
{
public:
	DFA();
	explicit DFA(const std::string &file_path);
	virtual ~DFA();

	[[nodiscard]]
	bool accepts(const std::string& string) const;

	[[nodiscard]]
	json to_json() const;

	void print() const;

	// deletes DFA on destruct
	void addState(DFA_State* state) { states.push_back(state); }
	void addTransition(const std::string &fromState, const std::string &toState, const symbol& onSymbol);

private:
	void validateAlphabetAndStore(const nlohmann::basic_json<> &alphabet_array);
	void validateStatesAndStore(const nlohmann::basic_json<> &states_array);
	void validateTransitionsAndStore(const nlohmann::basic_json<> &transitions_array);

private:
	std::vector<symbol> alphabet;
	std::vector<DFA_State*> states;
	DFA_State* startingState{};
};


#endif //DFA_DFA_H
