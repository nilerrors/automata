#include <iostream>
#include <fstream>

#include "DFA.h"
#include "NFA.h"
#include "ENFA.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

bool compareSrcJSON(const string &src, const json &j2)
{
	ifstream input_file(src);
	json j1;
	input_file >> j1;
	return j1 == j2;
}

void testDFA()
{
	DFA dfa;
	if (!dfa.accepts(""))
		throw runtime_error("Failed test 1: did not accept empty string");
	if (!dfa.accepts("0010110100"))
		throw runtime_error("Failed test 2: did not accept 0010110100");
	if (dfa.accepts("0001"))
		throw runtime_error("Failed test 3: did not accept 0001");

	dfa.clear();
	dfa.fromPath("DFA.json");
	if (dfa.accepts("0010110100"))
		throw runtime_error("Failed test 4: did not accept 0010110100");
	if (!dfa.accepts("0001"))
		throw runtime_error("Failed test 5: did not accept 0001");
	if (!compareSrcJSON("DFA.json", dfa.to_json()))
		throw runtime_error("Failed test 6: JSONs are not equal");
}

void testNFA()
{
	NFA nfa("input-ssc1.json");
	if (!compareSrcJSON("expected_output-ssc1.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 0: JSONs are not equal");

	nfa.clear();
	nfa.fromPath("input-ssc2.json");
	if (!compareSrcJSON("expected_output-ssc2.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 1: JSONs are not equal");
}

void testENFA()
{
	ENFA enfa("input-mssc1.json");
	enfa.toDFA().print();
}

int main() {
	testDFA();
	testNFA();
	testENFA();

	cout << "All tests passed" << endl;
	return 0;
}
