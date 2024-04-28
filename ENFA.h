//
// Created by nilerrors on 3/12/24.
//

#ifndef ENFA_H
#define ENFA_H

#include "NFA.h"


class ENFA : public NFA
{
public:
    ENFA();

    explicit ENFA(const std::string &file_path);

    virtual ~ENFA();

    void optimizeStart();

    void optimizeAccept();

    // union
    static void
    join(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &first, const std::shared_ptr<ENFA> &second);

    // concatenation
    static void
    link(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &first, const std::shared_ptr<ENFA> &second);

    // kleene star
    static void star(std::shared_ptr<ENFA> &result, const std::shared_ptr<ENFA> &enfa);
};


#endif //ENFA_H
