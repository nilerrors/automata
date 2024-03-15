#include <iostream>
#include <fstream>
#include <iomanip>

#include "DFA.h"
#include "NFA.h"
#include "ENFA.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

bool compareSrcJSON(const string &src, json j2)
{
	ifstream input_file(src);
	json j1;
	input_file >> j1;

	std::sort(j1["alphabet"].begin(), j1["alphabet"].end());
	std::sort(j1["states"].begin(), j1["states"].end(), [](const json &a, const json &b) {
		return a["name"] < b["name"];
	});
	std::sort(j1["transitions"].begin(), j1["transitions"].end(), [](const json &a, const json &b) {
		if (a["from"] == b["from"])
		{
			if (a["input"] == b["input"])
				return a["to"] < b["to"];
			return a["input"] < b["input"];
		}
		return a["from"] < b["from"];
	});

	std::sort(j2["alphabet"].begin(), j2["alphabet"].end());
	std::sort(j2["states"].begin(), j2["states"].end(), [](const json &a, const json &b) {
		return a["name"] < b["name"];
	});
	std::sort(j2["transitions"].begin(), j2["transitions"].end(), [](const json &a, const json &b) {
		if (a["from"] == b["from"])
		{
			if (a["input"] == b["input"])
				return a["to"] < b["to"];
			return a["input"] < b["input"];
		}
		return a["from"] < b["from"];
	});

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
		throw runtime_error("Failed test 6: DFA is not equal to the original JSON");
}

void testNFA()
{
	NFA nfa("input-ssc1.json");
	if (!compareSrcJSON("expected_output-ssc1.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 0: SSC1 is incorrect");

	nfa.clear();
	nfa.fromPath("input-ssc2.json");
	if (!compareSrcJSON("expected_output-ssc2.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 1: SSC2 is incorrect");
}

void testENFA()
{
	ENFA enfa("input-mssc1.json");
	if (!compareSrcJSON("expected_output-mssc1.json", enfa.toDFA().to_json()))
		throw runtime_error("Failed test 0: ENFA is incorrect");
}

void testProduct()
{
	DFA dfa1("input-product-and1.json");
    DFA dfa2("input-product-and2.json");
    DFA product(dfa1,dfa2,true); // true betekent doorsnede, false betekent unie
    if (!compareSrcJSON("expected_output-product.json", product.to_json()))
    {
    	product.print();
	    throw runtime_error("Failed test 0: Product Automaton is incorrect");
    }
}

int main() {
	testDFA();
	testNFA();
	testENFA();
	testProduct();

	cout << "All tests passed" << endl;
	return 0;
}
