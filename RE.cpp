//
// Created by nilerrors on 3/17/24.
//

#include "RE.h"

#include <deque>
#include <stack>
#include <utility>


RExpression::RExpression(const std::string &regex, const Symbol epsilon)
{
	if (!RE::isValid(regex))
		throw std::runtime_error("Invalid regex");

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
			value = epsilon;
			return;
		}
		type = SYMBOL;
		value = regex;
		return;
	}

	value = regex;

	size_t curidx = 0;
	const size_t len = regex.size();
	std::deque<RExpression *> stack;

	auto get_next_char = [&regex](size_t index) -> char {
		if (index + 1 >= regex.size())
			return '\0';
		return regex[index + 1];
	};

	while (curidx < len)
	{
		char c = regex[curidx];

		switch (c)
		{
			case '(':
			{
				size_t start = curidx;
				size_t end = start + 1;
				std::string sub;
				int count = 1;

				while (count != 0)
				{
					if (regex[end] == '(')
						count++;
					else if (regex[end] == ')')
						count--;
					if (count != 0)
						sub.push_back(regex[end]);
					end++;
				}

				auto *subexp = new RExpression(sub, epsilon);
				subexp->value = "(" + sub + ")";
				stack.push_front(subexp);

				curidx = end;
				break;
			}
			case '*':
			{
				RExpression *self = stack.front();
				stack.pop_front();

				auto *starexp = new RExpression(STAR);
				starexp->value = self->value + "*";
				starexp->left = self;

				if (stack.empty())
				{
					stack.push_front(starexp);
				}
				else
				{
					RExpression *left = stack.front();
					stack.pop_front();
					if (left->type == EPSILON)
						throw std::runtime_error("Can't concatenate with epsilon");

					auto *concatexp = new RExpression(CONCATENATION);
					concatexp->value = left->value + starexp->value;
					concatexp->left = left;
					concatexp->right = starexp;
					stack.front();
					stack.push_front(concatexp);
				}

				curidx++;
				break;
			}
			case '+':
			{
				// the union operation is the primary operation
				// this will be the operation for the whole regex

				type = UNION;
				left = stack.front();
				stack.pop_front();
				right = new RExpression(regex.substr(curidx + 1), epsilon);

				curidx = len;
				break;
			}
			default:
			{
				if (c == epsilon)
				{
					auto *epsilonexp = new RExpression(EPSILON);
					if (!stack.empty())
					{
						throw std::runtime_error("Can't concatenate with epsilon");
					}
					stack.push_front(epsilonexp);
					curidx++;
					break;
				}

				auto *symexp = new RExpression(SYMBOL);
				symexp->value = std::string(1, regex[curidx]);

				if (stack.empty() || get_next_char(curidx) == '*')
					stack.push_front(symexp);
				else
				{
					if (stack.front()->type == EPSILON)
						throw std::runtime_error("Can't concatenate with epsilon");

					RExpression *left = stack.front();
					stack.pop_front();
					auto *concatexpr = new RExpression(CONCATENATION);
					concatexpr->value = left->value + symexp->value;
					concatexpr->right = symexp;
					concatexpr->left = left;
					stack.push_front(concatexpr);
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
			throw std::runtime_error("Invalid regex");
	}
	else if (stack.size() == 1)
	{
		// has to be a single operation
		//		- empty
		//		- epsilon
		//		- symbol
		//		- star
		//		- a single concatenation
		RExpression *top = stack.front();
		type = top->type;
		value = regex;
		left = top->left;
		right = top->right;
		stack.pop_front();
	}
	else
		throw std::runtime_error("Invalid regex");
}

RExpression::~RExpression()
{
	delete left;
	delete right;
}

// Throws an exception if the regex is invalid
RE::RE(std::string regex, const Symbol epsilon) : regex(std::move(regex)), epsilon(epsilon)
{
	if (!isValid(regex))
		throw std::runtime_error("Invalid regex");
}

RE::~RE() = default;

ENFA RE::toENFA() const
{
	ENFA enfa;
	enfa.setEpsilon(epsilon);
	enfa.setAlphabet(getAlphabet(regex, epsilon));

	// TODO: Conversion from regex to ENFA

	return enfa;
}

std::set<Symbol> RE::getAlphabet(const std::string &regex, const Symbol epsilon)
{
	std::set<Symbol> alphabet;
	for (char c : regex)
	{
		if (c != '(' && c != ')' && c != '+' && c != '*' && c != epsilon)
			alphabet.insert(c);
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
			return '\0';
		return final_string[final_string.size() - 1];
	};

	auto get_next_char = [&regex](size_t index) -> char {
		if (index >= regex.size())
			return '\0';
		return regex[index];
	};

	for (char c : regex)
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
					return false;
				parenthesis.pop();
				final_string.push_back(')');
				break;
			}
			case '+':
			{
				if (get_prev_char() == '('
					|| get_prev_char() == '+'
					|| get_prev_char() == '\0')
					return false;
				if (get_next_char(final_string.size() + 1) == ')'
					|| get_next_char(final_string.size() + 1) == '+'
					|| get_next_char(final_string.size() + 1) == '*'
					|| get_next_char(final_string.size() + 1) == '\0')
					return false;
				final_string.push_back('+');
				break;
			}
			case '*':
			{
				if (get_prev_char() == '('
					|| get_prev_char() == '*'
					|| get_prev_char() == '+'
					|| get_prev_char() == '\0')
					return false;
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


