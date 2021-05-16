#pragma once
#include "lexical_analyzer.h"
#include "variables_manager.h"

class ExpressionAnalizer {
private:
	LexicalAnalyzer* lexicalAnalyzer;
	VariablesStore* variablesStore;

public:
	ExpressionAnalizer(LexicalAnalyzer* lexicalAnalyzer, VariablesStore* variablesStore) {
		this->lexicalAnalyzer = lexicalAnalyzer;
		this->variablesStore = variablesStore;
	}
	
};