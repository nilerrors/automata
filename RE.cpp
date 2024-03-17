//
// Created by nilerrors on 3/17/24.
//

#include "RE.h"

#include <stack>
#include <utility>

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


