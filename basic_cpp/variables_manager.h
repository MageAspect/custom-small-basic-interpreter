#pragma once
#include "boost\variant.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Variable {
	Variable(string name, boost::variant<int, double> value) {
		this->name = name;
		this->value = value;
	}
	Variable() {}
	string name;
	boost::variant<int, double> value;
};

class VariableNotFoundException : public exception {};
class VariableWrongNameException : public exception {};
class VariableDublicateException : public exception {};

class VariablesStore
{
private:
	vector<Variable> *varsStore = new vector<Variable>();

public:
	Variable getVariableByName(string name) {
		for (Variable var : *this->varsStore) {
			if (var.name == name) {
				return var;
			}
		}

		throw VariableNotFoundException();
	}

	void setVariable(Variable varToSet) {
		for (Variable &var : *this->varsStore) {
			if (var.name == varToSet.name) {
				var.value = varToSet.value;
				return;
			}
		}
		
		throw VariableNotFoundException();
	}

	void addVariable(Variable varToAdd) {
		if (varToAdd.name == "" || !isalpha(varToAdd.name[0])) {
			throw VariableWrongNameException();
		}
		for (Variable var : *this->varsStore) {
			if (var.name == varToAdd.name) {
				throw VariableDublicateException();
			}
		}
		this->varsStore->push_back(varToAdd);
	}

};



