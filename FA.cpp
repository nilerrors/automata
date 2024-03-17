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

FA::~FA()
{
	clear();
}

void FA::clear()
{
	alphabet.clear();
	for (const State *state : states)
	{
		delete state;
	}
	for (const Transition *transition : transitions)
	{
		delete transition;
	}

	startingState = nullptr;
	alphabet.clear();
	states.clear();
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
	if (allowEpsilonTransitions)
	{
		if (j["eps"].empty())
			throw std::runtime_error("Epsilon transitions are allowed, but no symbol was provided");
		if (!j["eps"].is_string())
			throw std::runtime_error("Epsilon symbol must be of type string");

		epsilon = j["eps"].get<std::string>().front();
	}

	validateAlphabetAndStore(j["alphabet"]);
	validateStatesAndStore(j["states"]);
	validateTransitionsAndStore(j["transitions"]);
}

void FA::addState(State *state)
{
	for (const auto s : states)
	{
		if (s->name == state->name)
			return;
	}
	states.insert(state);
}
void FA::addTransition(Transition *transition)
{
	for (const auto t : transitions)
	{
		if (t->from->name == transition->from->name
			&& t->to->name == transition->to->name
			&& t->symbol == transition->symbol)
			return;
	}
	transitions.insert(transition);
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

std::string FA::to_dot() const
{
	std::string result = "digraph " + type + " {\n";
	result += "  rankdir=LR;\n";
	for (const auto &state : states)
	{
		result += "  " + state->name + " [shape=" + (state->accepting ? "doublecircle" : "circle") + "];\n";
		if (state->starting)
			result += "  start -> " + state->name + ";\n";
	}
	for (const auto &transition : transitions)
	{
		result += "  " + transition->from->name + " -> " + transition->to->name
				+ " [label=\"" + transition->symbol + "\"];\n";
	}
	result += "}\n";
	return result;
}

void FA::print() const
{
	std::cout << std::setw(4) << to_json() << std::endl;
}

void FA::printStats() const
{
	std::cout << "no_of_states=" << states.size() << std::endl;

	auto symbol_count = [&](const Symbol symbol) -> long {
		return std::count_if(transitions.begin(), transitions.end(), [&symbol](const Transition *transition) {
			return transition->symbol == symbol;
		});
	};

	if (allowEpsilonTransitions)
		std::cout << "no_of_transitions[" << epsilon << "]=" << symbol_count(epsilon) << std::endl;
	for (const Symbol symbol : alphabet)
	{
		std::cout << "no_of_transitions[" << symbol << "]=" << symbol_count(symbol) << std::endl;
	}

	// the degree of a state is the number of transitions that go out of it
	std::map<uint, uint> degrees;
	for (const auto state : states)
	{
		uint degree = std::count_if(transitions.begin(), transitions.end(), [state](const Transition *transition) {
			return transition->from == state;
		});
		if (degrees.find(degree) == degrees.end())
			degrees[degree] = 0;
		degrees[degree]++;
	}

	for (const auto& [degree, count] : degrees)
	{
		std::cout << "degree[" << degree << "]=" << count << std::endl;
	}
}


void FA::validateAlphabetAndStore(const nlohmann::basic_json<> &alphabet_array)
{
	for (const auto& letter : alphabet_array)
	{
		if (!letter.is_string())
			throw std::runtime_error("symbol must be of type string");
		if (letter.size() != SYMBOL_SIZE)
			throw std::runtime_error("symbol must be of size 1");

		alphabet.insert(letter.get<std::string>().front());
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
			if (startingState != nullptr)
				throw std::runtime_error("cannot have multiple instances of starting states");

			startingState = new_state;
		}
	}

	if (startingState == nullptr)
		throw std::runtime_error("no starting state provided");
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
		{
			if (allowEpsilonTransitions && transition["input"].get<std::string>().front() != epsilon)
				throw std::runtime_error(
						"transition input attribute must be part of alphabet, got: "
							+ transition["input"].get<std::string>());
		}

		addTransition(
				new Transition(
						getState(transition["from"].get<std::string>()),
						getState(transition["to"].get<std::string>()),
						transition["input"].get<std::string>().front()));
	}
}


State *FA::getState(const std::string &name) const
{
	for (const auto state : states)
	{
		if (state->name == name)
			return state;
	}
	return nullptr;
}

State *FA::getNextState(const State *from, const Symbol symbol) const
{
	for (const auto transition : transitions)
	{
		if (transition->from == from && transition->symbol == symbol)
		return transition->to;
	}
	return nullptr;
}

SetOfStates *FA::getNextStates(const SetOfStates *from, const Symbol symbol) const
{
	auto *nextStates = new SetOfStates({});
	for (const auto state : from->states)
	{
		for (const auto transition : transitions)
		{
			if (transition->from == state && transition->symbol == symbol)
			nextStates->add(e_closure(transition->to, nextStates));
		}
	}
	return nextStates;
}


// modifies the given set of states
SetOfStates *FA::e_closure(State *state, SetOfStates *states) const
{
	states->add(state);
	for (const auto transition : transitions)
	{
		if (transition->from == state && transition->symbol == epsilon && !states->states.count(transition->to))
			e_closure(transition->to, states);
	}
	return states;
}
