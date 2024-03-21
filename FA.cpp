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
	for (auto &state : states)
	{
		delete state;
		state = nullptr;
	}
	for (auto &transition : transitions)
	{
		delete transition;
		transition = nullptr;
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
	if (state->starting)
	{
		if (startingState != nullptr)
			throw std::runtime_error("cannot have multiple instances of starting states");
		startingState = state;
	}
	for (const auto s : states)
	{
		if (s->name == state->name)
			return;
	}
	states.push_back(state);
}
void FA::addTransition(Transition *transition)
{
	if (transition == nullptr || transition->from == nullptr || transition->to == nullptr)
		return;
	for (const auto t : transitions)
	{
		if (t->from->name == transition->from->name
			&& t->to->name == transition->to->name
			&& t->symbol == transition->symbol)
			return;
	}
	transitions.push_back(transition);
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

void FA::to_json(const std::string &filename, bool format) const
{
	std::ofstream output_file(filename);
	output_file << std::setw(format ? 4 : 0) << to_json() << std::endl;
	output_file.close();
}

void FA::to_dot(const std::string &file) const
{
	std::ofstream output_file(file);
	output_file << to_dot();
	output_file.close();
}

std::string FA::to_dot() const
{
	std::string result = "digraph " + type + " {\n";
	result += "  rankdir=LR;\n";
	for (const auto &state : states)
	{
		result += "  \"" + state->name + "\" [shape=" + (state->accepting ? "doublecircle" : "circle") + "];\n";
		if (state->starting)
			result += "  start -> \"" + state->name + "\";\n";
	}
	for (const auto &transition : transitions)
	{
		result += "  \"" + transition->from->name + "\" -> \"" + transition->to->name
				+ "\" [label=\"" + transition->symbol + "\"];\n";
	}
	result += "}\n";
	return result;
}

std::string FA::to_stats() const
{
	std::string stats;

	auto symbol_count = [&](const Symbol symbol) -> long {
		return std::count_if(transitions.begin(), transitions.end(), [&symbol](const Transition *transition) {
			return transition->symbol == symbol;
		});
	};

	std::vector<Symbol> alphabet_symbols;
	std::copy(alphabet.begin(), alphabet.end(), std::back_inserter(alphabet_symbols));
	std::sort(alphabet_symbols.begin(), alphabet_symbols.end());

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

	std::vector<uint> degrees_vector;
	std::transform(degrees.begin(), degrees.end(), std::back_inserter(degrees_vector),
	               [](const std::pair<uint, uint> &degree) { return degree.first; });
	std::sort(degrees_vector.begin(), degrees_vector.end());


	stats += "no_of_states=" + std::to_string(states.size()) + "\n";

	if (allowEpsilonTransitions)
		stats += "no_of_transitions[" + std::string(1, epsilon) + "]=" + std::to_string(symbol_count(epsilon)) + "\n";

	for (const Symbol symbol : alphabet_symbols)
	{
		stats += "no_of_transitions[" + std::string(1, symbol) + "]=" + std::to_string(symbol_count(symbol)) + "\n";
	}

	for (const auto& degree : degrees_vector)
	{
		stats += "degree[" + std::to_string(degree) + "]=" + std::to_string(degrees[degree]) + "\n";
	}

	return stats;
}

void FA::print() const
{
	std::cout << std::setw(4) << to_json() << std::endl;
}

void FA::printStats() const
{
	std::cout << to_stats();
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


std::vector<State *> FA::getAcceptingStates() const
{
	std::vector<State *> acceptStates;
	for (auto state : states)
	{
		if (state->accepting)
			acceptStates.push_back(state);
	}
	return acceptStates;
}

std::vector<Transition *> FA::getTransitionsFromState(const State *state) const
{
	std::vector<Transition *> transitionFromState;
	for (const auto transition : transitions)
	{
		if (transition->from == state)
			transitionFromState.push_back(transition);
	}
	return transitionFromState;
}

std::vector<Transition *> FA::getTransitionsToState(const State *state) const
{
	std::vector<Transition *> transitionFromState;
	for (const auto transition : transitions)
	{
		if (transition->to == state)
			transitionFromState.push_back(transition);
	}
	return transitionFromState;
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
