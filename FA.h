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
    std::set<std::shared_ptr<State>> states;
    std::shared_ptr<State> state = nullptr;
    bool isStarting = false;

    explicit SetOfStates(const bool starting = false) : states({}), isStarting(starting)
    {
    }

    explicit SetOfStates(const std::set<std::shared_ptr<State>> &states, const bool starting = false)
            : states(states), isStarting(starting)
    {
    }

    void add(const std::shared_ptr<State> &s)
    {
        states.insert(s);
    }

    void add(const std::shared_ptr<SetOfStates> &set)
    {
        states.insert(set->states.begin(), set->states.end());
    }

    std::shared_ptr<State> to_state()
    {
        if (state != nullptr)
        {
            return state;
        }
        state = std::make_shared<State>();
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
                [](const std::shared_ptr<State> &s) -> bool { return s->accepting; });
    }

    [[nodiscard]]
    std::string to_string() const
    {
        std::string result = "{";
        std::vector<std::string> all_names;
        all_names.reserve(states.size());
        for (const std::shared_ptr<State> &s: states)
        {
            all_names.push_back(s->name);
        }
        std::sort(all_names.begin(), all_names.end());
        for (const std::string &name: all_names)
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
    std::pair<std::shared_ptr<State>, std::shared_ptr<State>> states;
    std::shared_ptr<State> state = nullptr;
    bool isStarting = false;
    bool isAccepting = false;

    PairOfStates() : states({nullptr, nullptr})
    {
    }

    PairOfStates(const std::shared_ptr<State> &first, const std::shared_ptr<State> &second, const bool start,
                 const bool end)
            : states({first, second}), isStarting(start), isAccepting(end)
    {
    }

    [[nodiscard]]
    std::shared_ptr<State> to_state()
    {
        if (state != nullptr)
        {
            return state;
        }
        state = std::make_shared<State>();
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
    std::shared_ptr<State> from;
    std::shared_ptr<State> to;
    Symbol symbol;

    Transition(const std::shared_ptr<State> &f, const std::shared_ptr<State> &t, const Symbol sym)
    {
        from = f;
        to = t;
        symbol = sym;
    }

    Transition(const std::shared_ptr<State> &f, const std::shared_ptr<State> &t, const std::string &sym)
    {
        from = f;
        to = t;
        symbol = sym.front();
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

    void fromJSON(const nlohmann::json &j);

    [[nodiscard]]
    virtual bool accepts(const std::string &string) const = 0;

    [[nodiscard]]
    virtual nlohmann::json to_json() const;

    void to_json(const std::string &filename, bool format = true) const;

    [[nodiscard]]
    virtual std::string to_dot() const;

    void to_dot(const std::string &file) const;

    [[nodiscard]]
    virtual std::string to_stats() const;

    void print() const;

    void printStats() const;

    void addState(const std::shared_ptr<State> &state);

    void addTransition(const std::shared_ptr<Transition> &transition);

    [[nodiscard]]
    const std::string &getType() const;

    [[nodiscard]]
    const std::set<Symbol> &getAlphabet() const;

    [[nodiscard]]
    Symbol getEpsilon() const;

    void setAlphabet(const std::set<Symbol> &alphabet);

    void setEpsilon(Symbol epsilon);

    [[nodiscard]]
    std::shared_ptr<State> getStartingState() const;

    [[nodiscard]]
    std::shared_ptr<SetOfStates> getStartingStates() const;

    [[nodiscard]]
    std::vector<std::shared_ptr<State>> getAcceptingStates() const;

    [[nodiscard]]
    const std::vector<std::shared_ptr<State>> &getStates() const;

    [[nodiscard]]
    const std::vector<std::shared_ptr<Transition>> &getTransitions() const;

    [[nodiscard]]
    std::vector<std::shared_ptr<Transition>> getTransitionsFromState(const std::shared_ptr<State> &state) const;

    [[nodiscard]]
    std::vector<std::shared_ptr<Transition>> getTransitionsToState(const std::shared_ptr<State> &state) const;

    [[nodiscard]]
    std::shared_ptr<State> getState(const std::string &name) const;

    [[nodiscard]]
    std::shared_ptr<State> getNextState(const std::shared_ptr<State> &from, Symbol symbol) const;

    [[nodiscard]]
    std::shared_ptr<SetOfStates> getNextStates(const std::shared_ptr<SetOfStates> &from, Symbol symbol) const;

    // modifies the given set of states
    std::shared_ptr<SetOfStates> e_closure(const std::shared_ptr<State> &state, const std::shared_ptr<SetOfStates> &states) const;

protected:
    void validateAlphabetAndStore(const nlohmann::json &alphabet_array);

    void validateStatesAndStore(const nlohmann::json &states_array);

    void validateTransitionsAndStore(const nlohmann::json &transitions_array);

protected:
    std::string type;
    std::set<Symbol> alphabet;
    std::vector<std::shared_ptr<State>> states;
    std::vector<std::shared_ptr<Transition>> transitions;
    std::shared_ptr<State> startingState = nullptr;

    bool allowEpsilonTransitions = false;
    // if epsilon is not used, it will be '\0'
    Symbol epsilon = '\0';
};


#endif //AUTOMATA_FA_H
