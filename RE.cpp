//
// Created by nilerrors on 3/17/24.
//

#include "RE.h"

#include <deque>
#include <iostream>
#include <stack>
#include <utility>


RExpression::RExpression(const std::string &regex, const Symbol epsilon)
{
    if (!RE::isValid(regex))
    {
        throw std::runtime_error("Invalid regex");
    }

    value = regex;

    if (regex.empty())
    {
        type = EMPTY;
        return;
    }
    if (regex.size() == 1)
    {
        if (regex[0] == epsilon)
        {
            type = EPSILON;
            return;
        }
        type = SYMBOL;
        return;
    }

    size_t curidx = 0;
    const size_t len = regex.size();
    std::stack<std::shared_ptr<RExpression>> stack;

    while (curidx < len)
    {
        switch (regex[curidx])
        {
        case '(':
        {
            const size_t start = curidx;
            size_t end = start + 1;
            std::string sub;
            int count = 1;

            while (count != 0)
            {
                if (regex[end] == '(')
                {
                    count++;
                }
                else if (regex[end] == ')')
                {
                    count--;
                }
                if (count != 0)
                {
                    sub.push_back(regex[end]);
                }
                end++;
            }

            std::shared_ptr<RExpression> subexp = std::make_shared<RExpression>(sub, epsilon);
            if (stack.empty() || (end < len && regex.at(end) == '*'))
            {
                stack.push(subexp);
            }
            else
            {
                std::shared_ptr<RExpression> left = stack.top();
                stack.pop();
                std::shared_ptr<RExpression> concatexpr = std::make_shared<RExpression>(CONCATENATION);
                concatexpr->value = left->value + subexp->value;
                concatexpr->left = left;
                concatexpr->right = subexp;
                stack.push(concatexpr);
            }

            curidx = end;
            break;
        }
        case '*':
        {
            std::shared_ptr<RExpression> self = stack.top();
            stack.pop();

            std::shared_ptr<RExpression> starexp = std::make_shared<RExpression>(STAR);
            starexp->value = self->value;
            starexp->left = self;

            if (stack.empty())
            {
                stack.push(starexp);
            }
            else
            {
                std::shared_ptr<RExpression> left = stack.top();
                stack.pop();
                if (left->type == EPSILON)
                {
                    throw std::runtime_error("Can't concatenate with epsilon");
                }

                std::shared_ptr<RExpression> concatexp = std::make_shared<RExpression>(CONCATENATION);
                concatexp->value = left->value + starexp->value;
                concatexp->left = left;
                concatexp->right = starexp;
                stack.push(concatexp);
            }

            curidx++;
            break;
        }
        case '+':
        {
            // the union operation is the primary operation
            // this will be the operation for the whole regex

            type = UNION;
            left = stack.top();
            stack.pop();
            right = std::make_shared<RExpression>(regex.substr(curidx + 1), epsilon);

            curidx = len;
            break;
        }
        default:
        {
            if (regex[curidx] == epsilon)
            {
                std::shared_ptr<RExpression> epsilonexp = std::make_shared<RExpression>(EPSILON);
                if (!stack.empty())
                {
                    throw std::runtime_error("Can't concatenate with epsilon");
                }
                stack.push(epsilonexp);
                curidx++;
                break;
            }

            std::shared_ptr<RExpression> symexp = std::make_shared<RExpression>(SYMBOL);
            symexp->value = std::string(1, regex[curidx]);

            if (stack.empty() || regex.at(curidx) == '*')
            {
                stack.push(symexp);
            }
            else
            {
                if (stack.top()->type == EPSILON)
                {
                    throw std::runtime_error("Can't concatenate with epsilon");
                }

                std::shared_ptr<RExpression> left = stack.top();
                stack.pop();
                std::shared_ptr<RExpression> concatexpr = std::make_shared<RExpression>(CONCATENATION);
                concatexpr->value = left->value + symexp->value;
                concatexpr->right = symexp;
                concatexpr->left = left;
                stack.push(concatexpr);
            }

            curidx++;
            break;
        }
        }
    }

    if (stack.empty())
    {
        // has to be union
        if (type != UNION)
        {
            throw std::runtime_error("Invalid regex");
        }
    }
    else if (stack.size() == 1)
    {
        // has to be a single operation
        //		- empty
        //		- epsilon
        //		- symbol
        //		- star
        //		- a single concatenation
        const std::shared_ptr<RExpression> top = stack.top();
        type = top->type;
        value = regex;
        left = top->left;
        right = top->right;
        stack.pop();
    }
    else
    {
        throw std::runtime_error("Invalid regex");
    }
}

RExpression::~RExpression() = default;

std::shared_ptr<ENFA> RExpression::toENFA(const Symbol epsilon) const
{
    switch (type)
    {
    case EMPTY:
    {
        std::shared_ptr<ENFA> enfa = std::make_shared<ENFA>();
        enfa->setEpsilon(epsilon);
        enfa->setAlphabet({});
        return enfa;
    }
    case EPSILON:
    {
        std::shared_ptr<ENFA> enfa = std::make_shared<ENFA>();
        enfa->setEpsilon(epsilon);
        enfa->setAlphabet({});
        std::shared_ptr<State> start = std::make_shared<State>("startend", true, true);
        enfa->addState(start);
        return enfa;
    }
    case SYMBOL:
    {
        std::shared_ptr<ENFA> enfa = std::make_shared<ENFA>();
        enfa->setEpsilon(epsilon);
        enfa->setAlphabet({value.front()});
        std::shared_ptr<State> start = std::make_shared<State>("start", true, false);
        std::shared_ptr<State> end = std::make_shared<State>("end", false, true);
        enfa->addState(start);
        enfa->addState(end);
        enfa->addTransition(std::make_shared<Transition>(start, end, value.front()));
        return enfa;
    }
    case STAR:
    {
        if (left == nullptr)
        {
            throw std::runtime_error("Invalid regex");
        }
        std::shared_ptr<ENFA> enfa = std::make_unique<ENFA>();
        std::shared_ptr<ENFA> in_star = left->toENFA(epsilon);
        ENFA::star(enfa, in_star);
        return enfa;
    }
    case UNION:
    {
        if (left == nullptr || right == nullptr)
        {
            throw std::runtime_error("Invalid regex");
        }
        std::shared_ptr<ENFA> enfa = std::make_shared<ENFA>();
        std::shared_ptr<ENFA> left = this->left->toENFA(epsilon);
        std::shared_ptr<ENFA> right = this->right->toENFA(epsilon);
        ENFA::join(enfa, left, right);
        return enfa;
    }
    case CONCATENATION:
    {
        if (left == nullptr || right == nullptr)
        {
            throw std::runtime_error("Invalid regex");
        }
        std::shared_ptr<ENFA> enfa = std::make_shared<ENFA>();
        std::shared_ptr<ENFA> left = this->left->toENFA(epsilon);
        left->optimizeAccept();
        std::shared_ptr<ENFA> right = this->right->toENFA(epsilon);
        right->optimizeAccept();
        ENFA::link(enfa, left, right);
        return enfa;
    }
    default:
        throw std::runtime_error("Invalid regex");
    }
}

// Throws an exception if the regex is invalid
RE::RE(std::string regex, const Symbol epsilon) : regex(std::move(regex)), epsilon(epsilon)
{
    if (!isValid(regex))
    {
        throw std::runtime_error("Invalid regex");
    }
}

RE::~RE() = default;

ENFA RE::toENFA() const
{
    ENFA enfa;
    enfa.setEpsilon(epsilon);
    enfa.setAlphabet(getAlphabet(regex, epsilon));

    RExpression re = RExpression(regex, epsilon);

    std::shared_ptr<ENFA> temp = re.toENFA(epsilon);
    temp->optimizeAccept();
    enfa = *temp;
    return enfa;
}

std::set<Symbol> RE::getAlphabet(const std::string &regex, const Symbol epsilon)
{
    std::set<Symbol> alphabet;
    for (char c: regex)
    {
        if (c != '(' && c != ')' && c != '+' && c != '*' && c != epsilon)
        {
            alphabet.insert(c);
        }
    }
    return alphabet;
}

bool RE::isValid(const std::string &regex)
{
    std::stack<char> parenthesis;
    std::string final_string;
    final_string.reserve(regex.size());

    auto get_prev_char = [&final_string]() -> char {
        if (final_string.empty())
        {
            return '\0';
        }
        return final_string[final_string.size() - 1];
    };

    auto get_next_char = [&regex](const size_t index) -> char {
        if (index >= regex.size())
        {
            return '\0';
        }
        return regex[index];
    };

    for (const char c: regex)
    {
        switch (c)
        {
        case '(':
        {
            parenthesis.push('(');
            final_string.push_back('(');
            break;
        }
        case ')':
        {
            if (parenthesis.empty())
            {
                return false;
            }
            parenthesis.pop();
            final_string.push_back(')');
            break;
        }
        case '+':
        {
            if (get_prev_char() == '('
                || get_prev_char() == '+'
                || get_prev_char() == '\0')
            {
                return false;
            }
            if (get_next_char(final_string.size() + 1) == ')'
                || get_next_char(final_string.size() + 1) == '+'
                || get_next_char(final_string.size() + 1) == '*'
                || get_next_char(final_string.size() + 1) == '\0')
            {
                return false;
            }
            final_string.push_back('+');
            break;
        }
        case '*':
        {
            if (get_prev_char() == '('
                || get_prev_char() == '*'
                || get_prev_char() == '+'
                || get_prev_char() == '\0')
            {
                return false;
            }
            final_string.push_back('*');
            break;
        }
        default:
        {
            final_string.push_back(c);
            break;
        }
        }
    }

    return parenthesis.empty() && (regex == final_string);
}


