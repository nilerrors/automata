# Automata

## Description

A simple implementation of finite automata and regular expressions.

## Implementation

This project is a simple implementation of the following:
- Deterministic Finite Automata (DFA)
- Non-Deterministic Finite Automata (NFA)
- Epsilon Non-Deterministic Finite Automata (ε-NFA)
- Regular Expression (RE)

### It has the following functionalities:
- For all finite automata:
  - Read the automata from a file (JSON)
  - Print the automata
  - Convert the automata to JSON
  - Convert the automata to DOT (Graphviz)
  - Save the automata to a file (JSON, DOT)
  - Check if a string is accepted by the automata

- DFA
  - Check if a string is accepted by the DFA
  - Product of two DFAs
    - Union of two DFAs
    - Intersection of two DFAs
  - Convert the DFA to a RE (coming soon)
  - Minimize the DFA (using TFA, coming soon)

- NFA
  - Check if a string is accepted by the NFA (by converting it to a DFA)
  - Convert the NFA to a DFA

- ε-NFA
  - Check if a string is accepted by the ε-NFA (by converting it to a NFA)
  - Convert the ε-NFA to a NFA

- RE
  - Convert the RE to a ε-NFA


## Plans
-[ ] Convert a DFA to its corresponding RE using the state elimination
-[ ] ^ Minimize the DFA using the table-filling algorithm
