cmake_minimum_required(VERSION 3.27)
project(Automata)

set(CMAKE_CXX_STANDARD 17)

add_executable(Automata main.cpp
        DFA.cpp
        DFA.h
        FA.cpp
        FA.h
        NFA.cpp
        NFA.h
        ENFA.cpp
        ENFA.h
        RE.cpp
        RE.h
        StatesTable.cpp
        StatesTable.h)
