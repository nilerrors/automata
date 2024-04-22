//
// Created by nilerrors on 3/2/24.
//

#ifndef AUTOMATA_FA_H
#define AUTOMATA_FA_H

#include <string>
#include <set>
#include <utility>
#include "json.hpp"

using json = nlohmann::json;
using Symbol = char;

constexpr int SYMBOL_SIZE = 1;

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

    State(std::string name, const bool isBegin, const bool isEnd)
            : name(std::move(name)), starting(isBegin), accepting(isEnd)
    {
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
    std::set<State *> states;
    State *state = nullptr;
    bool isStarting = false;

    explicit SetOfStates(const std::set<State *> &states, const bool starting = false)
            : states(states), isStarting(starting)
    {
    }

    void add(State *state)
    {
        states.insert(state);
    }

    void add(const SetOfStates *set)
    {
        states.insert(set->states.begin(), set->states.end());
    }

    State *to_state()
    {
        if (state != nullptr)
        {
            return state;
        }
        state = new State();
        state->name = to_string();
        state->starting = isStarting;
        state->accepting = isAccepting();
        return state;
    }

    [[nodiscard]]
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
        std::vector<std::string> all_names;
        all_names.reserve(states.size());
        for (const auto state: states)
        {
            all_names.push_back(state->name);
        }
        std::sort(all_names.begin(), all_names.end());
        for (auto &name: all_names)
        {
            if (name != all_names.front())
            {
                result += ",";
            }
            result += name;
        }
        result += "}";
        return result;
    }
};

struct PairOfStates
{
    std::pair<State *, State *> states;
    State *state = nullptr;
    bool isStarting = false;
    bool isAccepting = false;

    PairOfStates() : states({nullptr, nullptr})
    {
    }

    PairOfStates(State *first, State *second, const bool start, const bool end)
            : states({first, second}), isStarting(start), isAccepting(end)
    {
    }

    [[nodiscard]]
    State *to_state()
    {
        if (state != nullptr)
        {
            return state;
        }
        state = new State();
        state->name = to_string();
        state->starting = isStarting;
        state->accepting = isAccepting;
        return state;
    }

    [[nodiscard]]
    std::string to_string() const
    {
        return "(" + states.first->name + "," + states.second->name + ")";
    }
};

struct Transition
{
    State *from;
    State *to;
    Symbol symbol;

    Transition(State *from, State *to, const Symbol symbol)
    {
        Transition::from = from;
        Transition::to = to;
        Transition::symbol = symbol;
    }

    Transition(State *from, State *to, const std::string &symbol)
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

class DFA;

class FA
{
public:
    explicit FA(const std::string &type);

    virtual ~FA();

    void clear();

    void fromPath(const std::string &file_path);

    void fromJSON(const json &j);

    [[nodiscard]]
    virtual bool accepts(const std::string &string) const = 0;

    [[nodiscard]]
    virtual json to_json() const;

    void to_json(const std::string &filename, bool format = true) const;

    [[nodiscard]]
    virtual std::string to_dot() const;

    void to_dot(const std::string &file) const;

    [[nodiscard]]
    virtual std::string to_stats() const;

    void print() const;

    void printStats() const;

    void addState(State *state);

    void addTransition(Transition *transition);

    [[nodiscard]] const std::string &getType() const
    {
        return type;
    }

    [[nodiscard]] const std::set<Symbol> &getAlphabet() const
    {
        return alphabet;
    }

    [[nodiscard]] State *getStartingState() const
    {
        return startingState;
    }

    [[nodiscard]] SetOfStates *getStartingStates() const
    {
        return e_closure(startingState, new SetOfStates({}, true));
    }

    [[nodiscard]] std::vector<State *> getAcceptingStates() const;

    [[nodiscard]] const std::vector<State *> &getStates() const
    {
        return states;
    }

    [[nodiscard]] const std::vector<Transition *> &getTransitions() const
    {
        return transitions;
    }

    [[nodiscard]] std::vector<Transition *> getTransitionsFromState(const State *state) const;

    [[nodiscard]] std::vector<Transition *> getTransitionsToState(const State *state) const;

    [[nodiscard]] State *getState(const std::string &name) const;

    [[nodiscard]] State *getNextState(const State *from, Symbol symbol) const;

    [[nodiscard]] SetOfStates *getNextStates(const SetOfStates *from, Symbol symbol) const;

    // modifies the given set of states
    SetOfStates *e_closure(State *state, SetOfStates *states) const;

    void setAlphabet(const std::set<Symbol> &alphabet)
    {
        FA::alphabet = alphabet;
    }

    void setEpsilon(const Symbol epsilon)
    {
        FA::epsilon = epsilon;
    }

protected:
    void validateAlphabetAndStore(const nlohmann::basic_json<> &alphabet_array);

    void validateStatesAndStore(const nlohmann::basic_json<> &states_array);

    void validateTransitionsAndStore(const nlohmann::basic_json<> &transitions_array);

protected:
    std::string type;
    std::set<Symbol> alphabet;
    std::vector<State *> states;
    std::vector<Transition *> transitions;
    State *startingState = nullptr;

    bool allowEpsilonTransitions = false;
    // if epsilon is not used, it will be '\0'
    Symbol epsilon = '\0';
};


#endif //AUTOMATA_FA_H
