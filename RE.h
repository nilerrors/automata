//
// Created by nilerrors on 3/17/24.
//

#ifndef RE_H
#define RE_H

#include "ENFA.h"
#include "FA.h"


enum RExpressionType
{
    EMPTY,                    // empty,					∅

    // RE A, RE S, Symbol a, Symbol b
    CONCATENATION,            // regex concatenation,		R.S
    UNION,                    // regex union,				R+S
    STAR,                     // kleene star,				R*
    SYMBOL,                   // symbols,					a
    EPSILON,                  // epsilon,					ε
};

class RExpression
{
public:
    RExpression(const std::string &regex, Symbol epsilon);

    explicit RExpression(RExpressionType type) : type(type)
    {
    }

    ~RExpression();

    [[nodiscard]] ENFA *toENFA(Symbol epsilon) const;

private:
    RExpressionType type = EMPTY;
    std::string value;
    RExpression *left = nullptr;
    RExpression *right = nullptr;
};


class RE
{
public:
    RE(std::string regex, Symbol epsilon);

    ~RE();

    [[nodiscard]] ENFA toENFA() const;

    static bool isValid(const std::string &regex);

    static std::set<Symbol> getAlphabet(const std::string &regex, Symbol epsilon);

private:
    std::string regex;
    Symbol epsilon;
};


#endif //RE_H
