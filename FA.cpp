//
// Created by nilerrors on 3/2/24.
//

#include "FA.h"
#include <fstream>
#include <iostream>
#include <iomanip>

FA::FA(const std::string &type)
{
	FA::type = type;
}

void FA::clear()
{
	alphabet.clear();
	for (State *&state : states)
	{
		delete state;
		state = nullptr;
	}
	for (Transition *&transition : transitions)
	{
		delete transition;
		transition = nullptr;
	}

	states.clear();
	startingStates.clear();
	transitions.clear();
}

void FA::fromPath(const std::string &file_path)
{
	std::ifstream input_file(file_path);
	json j;

	if (input_file.fail())
		throw std::runtime_error("file could not be found: " + file_path);

	input_file >> j;

	fromJSON(j);
}

void FA::fromJSON(const json &j)
{
	if (!j["type"].is_string() || j["type"] != type)
		throw std::runtime_error("Automata type should be '" + type + "'");
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

FA::~FA()
{
	for (State *&state : states)
	{
		delete state;
		state = nullptr;
	}
	for (Transition *&transition : transitions)
	{
		delete transition;
		transition = nullptr;
	}
}

json FA::to_json() const
{
	json j;

	j["type"] = "DFA";
	for (const auto& symbol : alphabet)
	{
		j["alphabet"].push_back(std::string().assign(1, symbol));
	}
	for (const auto &state : states)
	{
		j["states"].push_back(state->to_json());
	}
	for (const auto &transition : transitions)
	{
		j["transitions"].push_back(transition->to_json());
	}

	return j;
}

void FA::print() const
{
	std::cout << std::setw(4) << to_json() << std::endl;
}


void FA::validateAlphabetAndStore(const nlohmann::basic_json<> &alphabet_array)
{
	for (const auto& letter : alphabet_array)
	{
		if (!letter.is_string())
			throw std::runtime_error("symbol must be of type string");
		if (letter.size() != SYMBOL_SIZE)
			throw std::runtime_error("symbol must be of size 1");

		alphabet.push_back(letter.get<std::string>().front());
	}
}

void FA::validateStatesAndStore(const nlohmann::basic_json<> &states_array)
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

		auto* new_state = new State(
				state["name"].get<std::string>(),
				state["starting"].get<bool>(),
				state["accepting"].get<bool>());
		addState(new_state);

		if (new_state->starting)
		{
			if (!startingStates.empty() && !allowMultipleStartStates)
				throw std::runtime_error("cannot have multiple instances of starting states");

			startingStates.push_back(new_state);
		}
	}

	if (startingStates.empty())
		throw std::runtime_error("no starting state(s) provided");
}

void FA::validateTransitionsAndStore(const nlohmann::basic_json<> &transitions_array)
{
	for (const auto& transition : transitions_array)
	{
		if (!transition.is_object())
			throw std::runtime_error("transition must be of type object");
		if (transition["from"].empty())
			throw std::runtime_error("transition from must be given");
		if (!transition["from"].is_string())
			throw std::runtime_error("transition from must be of type string");
		if (getState(transition["from"].get<std::string>()) == nullptr)
			throw std::runtime_error("transition from attribute must be part of states");
		if (transition["to"].empty())
			throw std::runtime_error("transition to attribute must not be empty");
		if (!transition["to"].is_string())
			throw std::runtime_error("transition to attribute must be of type string");
		if (getState(transition["to"].get<std::string>()) == nullptr)
			throw std::runtime_error("transition to attribute must be part of states");
		if (transition["input"].empty())
			throw std::runtime_error("transition input attribute must not be empty");
		if (!transition["input"].is_string())
			throw std::runtime_error("transition input attribute must be of type string");
		if (transition["input"].size() != SYMBOL_SIZE)
			throw std::runtime_error("transition input attribute must be of size 1");
		if (std::find(alphabet.cbegin(), alphabet.cend(),
		              transition["input"].get<std::string>().front()) == alphabet.cend())
			throw std::runtime_error(
					"transition input attribute must be part of alphabet, got: "
					+ transition["input"].get<std::string>());

		addTransition(
				new Transition(
						getState(transition["from"].get<std::string>()),
						getState(transition["to"].get<std::string>()),
						transition["input"].get<std::string>().front()));
	}
}

