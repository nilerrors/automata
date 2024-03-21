#include <iostream>
#include <fstream>
#include <iomanip>

#include "DFA.h"
#include "NFA.h"
#include "ENFA.h"
#include "json.hpp"
#include "RE.h"

using namespace std;
using json = nlohmann::json;


void testDFA();
void testNFA();
void testENFA();
void testProduct();
void testREisValid();
void testRE();
void testRE2();
bool compareSrcJSON(const string &src, json j2);
void print_allocs();

int main() {
	testDFA();
	print_allocs();

	testNFA();
	print_allocs();

	testENFA();
	print_allocs();

	testProduct();
	print_allocs();

	testREisValid();
	print_allocs();

	testRE();
	print_allocs();

	testRE2();
	print_allocs();

	cout << "All tests passed" << endl;
	return 0;
}

////
///
// TESTS
///
////

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
	dfa.fromPath("jsons/DFA.json");
	if (dfa.accepts("0010110100"))
		throw runtime_error("Failed test 4: did not accept 0010110100");
	if (!dfa.accepts("0001"))
		throw runtime_error("Failed test 5: did not accept 0001");
	if (!compareSrcJSON("jsons/DFA.json", dfa.to_json()))
		throw runtime_error("Failed test 6: DFA is not equal to the original JSON");
}

void testNFA()
{
	NFA nfa("jsons/input-ssc1.json");
	if (!compareSrcJSON("jsons/expected_output-ssc1.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 0: SSC1 is incorrect");

	nfa.clear();
	nfa.fromPath("jsons/input-ssc2.json");
	if (!compareSrcJSON("jsons/expected_output-ssc2.json", nfa.toDFA().to_json()))
		throw runtime_error("Failed test 1: SSC2 is incorrect");
}

void testENFA()
{
	ENFA enfa("jsons/input-mssc1.json");
	if (!compareSrcJSON("jsons/expected_output-mssc1.json", enfa.toDFA().to_json()))
		throw runtime_error("Failed test 0: ENFA is incorrect");
}

void testProduct()
{
	DFA dfa1("jsons/input-product-and1.json");
    DFA dfa2("jsons/input-product-and2.json");
    DFA product(dfa1,dfa2,true); // true betekent doorsnede, false betekent unie
    if (!compareSrcJSON("jsons/expected_output-product.json", product.to_json()))
    {
    	product.print();
	    throw runtime_error("Failed test 0: Product Automaton is incorrect");
    }
}

void testREisValid()
{
	if (!RE::isValid(""))
		throw runtime_error("Failed test 0: RE is valid");
	if (!RE::isValid("this"))
		throw runtime_error("Failed test 1: RE is invalid");
	if (RE::isValid("(this"))
		throw runtime_error("Failed test 2: RE is valid");
	if (RE::isValid("this)"))
		throw runtime_error("Failed test 3: RE is valid");
	if (RE::isValid("+this"))
		throw runtime_error("Failed test 4: RE is valid");
	if (RE::isValid("this+"))
		throw runtime_error("Failed test 5: RE is valid");
	if (RE::isValid("this+*"))
		throw runtime_error("Failed test 6: RE is valid");
	if (RE::isValid("this+*+"))
		throw runtime_error("Failed test 7: RE is valid");
	if (!RE::isValid("(m+y)*+(e+y+m+i)s"))
		throw runtime_error("Failed test 8: RE is invalid");
	if (RE::isValid("(m+y)*+e+y+m+i)s"))
		throw runtime_error("Failed test 9: RE is valid");
	if (RE::isValid("(m+y)*+(e+y+m+is"))
		throw runtime_error("Failed test 10: RE is valid");
}

void testRE()
{
    RE re("(m+y)*+(e+y+m+i)s",'e');
    ENFA enfa = re.toENFA();

	std::string expected_stats =
		"no_of_states=26\n"
		"no_of_transitions[e]=26\n"
		"no_of_transitions[i]=1\n"
		"no_of_transitions[m]=2\n"
		"no_of_transitions[s]=1\n"
		"no_of_transitions[y]=2\n"
		"degree[0]=1\n"
		"degree[1]=18\n"
		"degree[2]=7\n";

    if (expected_stats != enfa.to_stats())
    	throw runtime_error("Failed test 0: stats are not equal");
    if (!enfa.accepts("ys"))
    	throw runtime_error("Failed test 1: text is not accepted");
    if (!enfa.accepts("mmyyymmmym"))
    	throw runtime_error("Failed test 2: text is not accepted");
    if (!enfa.accepts("s"))
    	throw runtime_error("Failed test 3: text is not accepted");
    if (enfa.accepts("ss"))
    	throw runtime_error("Failed test 4: text is accepted");
    if (enfa.accepts("ims"))
    	throw runtime_error("Failed test 5: text is accepted");
    if (enfa.accepts("mimis"))
    	throw runtime_error("Failed test 6: text is accepted");
}

void testRE2()
{
	RE re("(kd+x+k)(i)*(k+i)",'x');
	ENFA enfa = re.toENFA();

	std::string expected_stats =
		"no_of_states=22\n"
		"no_of_transitions[x]=20\n"
		"no_of_transitions[d]=1\n"
		"no_of_transitions[i]=2\n"
		"no_of_transitions[k]=3\n"
		"degree[0]=1\n"
		"degree[1]=16\n"
		"degree[2]=5\n";

    if (expected_stats != enfa.to_stats())
    	throw runtime_error("Failed test 0: stats are not equal:\ngot:\n" + enfa.to_stats() + "\nexpected:\n" + expected_stats + "\n");
	if (!enfa.accepts("kdk"))
    	throw runtime_error("Failed test 1: text is not accepted");
    if (!enfa.accepts("i"))
    	throw runtime_error("Failed test 2: text is not accepted");
    if (!enfa.accepts("kiiiiii"))
    	throw runtime_error("Failed test 3: text is not accepted");
    if (enfa.accepts("kikk"))
    	throw runtime_error("Failed test 4: text is accepted");
    if (enfa.accepts(""))
    	throw runtime_error("Failed test 5: text is accepted");
    if (enfa.accepts("kdiiiiiiki"))
    	throw runtime_error("Failed test 6: text is accepted");
}




////
///
// UTILS
///
////

size_t total_size = 0;
size_t total_allocs = 0;
size_t total_deacllocs = 0;

void *operator new(size_t size)
{
	void *p = malloc(size);
	if (!p)
		throw std::bad_alloc();
	total_size += size;
	total_allocs++;
	return p;
}

void operator delete(void *p) noexcept
{
	total_deacllocs++;
	free(p);
}

void print_allocs()
{
	std::cout << std::endl;
	std::cout << "Total size: " << total_size / 1000 << "kB" << std::endl;
	std::cout << "Total allocs: " << total_allocs << std::endl;
	std::cout << "Total deallocs: " << total_deacllocs << std::endl;
	if (total_allocs == total_deacllocs)
	{
		std::cout << "All memory deallocated" << std::endl;
	}
	else
	{
		std::cout << "Memory leaks" << std::endl;
	}

	total_size = 0;
	total_allocs = 0;
	total_deacllocs = 0;
}

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
