//
// Created by nilerrors on 2/24/24.
//

#include <fstream>
#include <iostream>
#include <iomanip>

#include "DFA.h"


DFA::DFA()
{
	alphabet = {"0", "1"};
	auto s0 = new DFA_State("s0", true, true);
	auto s1 = new DFA_State("s1");
	auto s2 = new DFA_State("s2");
	startingState = s0;
	s0->addTransition("0", s0);
	s0->addTransition("1", s1);
	s1->addTransition("0", s1);
	s1->addTransition("1", s0);
	s2->addTransition("0", s1);
	s2->addTransition("1", s2);
}

/*
 * Will throw runtime_error if data is invalid
 */
DFA::DFA(const std::string &file_path)
{
	std::ifstream input_file(file_path);
	json j;

	if (input_file.fail())
		throw std::runtime_error("file could not be found: " + file_path);

	input_file >> j;

	if (!j["type"].is_string() || j["type"] != "DFA")
		throw std::runtime_error("Automata type should be DFA");
	if (!j["alphabet"].is_array())
		throw std::runtime_error("Invalid alphabet type, must be of type array");
	if (!j["states"].is_array())
		throw std::runtime_error("Invalid states type, must be of type array");
	if (!j["transitions"].is_array())
		throw std::runtime_error("Invalid transitions type, must be of type array");

	validateAlphabetAndStore(j["alphabet"]);
	validateStatesAndStore(j["states"]);
	validateTransitionsAndStore(j["transitions"]);
}

DFA::~DFA()
{
	for (DFA_State*& state : states)
	{
		delete state;
		state = nullptr;
	}
}

void DFA::addTransition(const std::string &fromState, const std::string &toState, const symbol& onSymbol)
{
	DFA_State* from = nullptr;
	DFA_State* to = nullptr;
	for (auto state : states)
	{
		if (state->name == fromState)
			from = state;
		if (state->name == toState)
			to = state;
	}
	if (from == nullptr)
		throw std::runtime_error("state from attribute cannot be found: " + fromState);
	if (to == nullptr)
		throw std::runtime_error("state to attribute cannot be found: " + toState);
	from->addTransition(onSymbol, to);
}

bool DFA::accepts(const std::string& string) const
{
	DFA_State* currentState = startingState;
	if (currentState == nullptr)
		return false;

	for (char c : string)
	{
		currentState = currentState->getState(std::string().assign(1, c));
		if (currentState == nullptr)
			return false;
	}
	return currentState->accepting;
}

json DFA::to_json() const
{
	json j;

	j["type"] = "DFA";
	for (const auto& symb : alphabet)
	{
		j["alphabet"].push_back(symb);
	}
	for (auto state : states)
	{
		j["states"].push_back(state->to_json());
		for (const auto& transition : state->transitions_to_json())
		{
			j["transitions"].push_back(transition);
		}
	}

	return j;
}

void DFA::print() const
{
	std::cout << std::setw(4) << to_json() << std::endl;
}

void DFA::validateAlphabetAndStore(const nlohmann::basic_json<>& alphabet_array)
{
	for (const auto& symb : alphabet_array)
	{
		if (!symb.is_string())
			throw std::runtime_error("symbol must be of type string");
		if (symb.size() != SYMBOL_SIZE)
			throw std::runtime_error("symbol must be of size 1");

		alphabet.push_back(symb.get<symbol>());
	}
}

void DFA::validateStatesAndStore(const nlohmann::basic_json<>& states_array)
{
	for (const auto& state : states_array)
	{
		if (!state.is_object())
			throw std::runtime_error("state must be of type object");
		if (state["name"].empty())
			throw std::runtime_error("state name must be given");
		if (!state["name"].is_string())
			throw std::runtime_error("state name must be of type string");
		if (state["starting"].empty())
			throw std::runtime_error("state starting attribute must not be empty");
		if (!state["starting"].is_boolean())
			throw std::runtime_error("state starting attribute must be of type boolean");
		if (state["accepting"].empty())
			throw std::runtime_error("state accepting attribute must not be empty");
		if (!state["accepting"].is_boolean())
			throw std::runtime_error("state accepting attribute must be of type boolean");

		auto new_state = new DFA_State(
								state["name"].get<std::string>(),
								state["starting"].get<bool>(),
								state["accepting"].get<bool>());
		addState(new_state);

		if (new_state->starting)
		{
			if (startingState != nullptr)
				throw std::runtime_error("cannot have multiple instances of starting states");

			startingState = new_state;
		}
	}

	if (startingState == nullptr)
		throw std::runtime_error("no starting state provided");
}

void DFA::validateTransitionsAndStore(const nlohmann::basic_json<> &transitions_array)
{
	for (const auto& transition : transitions_array)
	{
		if (!transition.is_object())
			throw std::runtime_error("transition must be of type object");
		if (transition["from"].empty())
			throw std::runtime_error("transition from must be given");
		if (!transition["from"].is_string())
			throw std::runtime_error("transition from must be of type string");
		if (transition["to"].empty())
			throw std::runtime_error("transition to attribute must not be empty");
		if (!transition["to"].is_string())
			throw std::runtime_error("transition to attribute must be of type string");
		if (transition["input"].empty())
			throw std::runtime_error("transition input attribute must not be empty");
		if (!transition["input"].is_string())
			throw std::runtime_error("transition input attribute must be of type string");
		if (transition["input"].size() != SYMBOL_SIZE)
			throw std::runtime_error("transition input attribute must be of size 1");
		if (std::find(alphabet.cbegin(), alphabet.cend(),
					  transition["input"].get<symbol>()) == alphabet.cend())
			throw std::runtime_error(
					"transition input attribute must be part of alphabet, got: "
					+ transition["input"].get<std::string>());

		addTransition(
				transition["from"].get<std::string>(),
				transition["to"].get<std::string>(),
				transition["input"].get<symbol>());
	}
}
