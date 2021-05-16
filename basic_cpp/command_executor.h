#pragma once
#include "lexical_analyzer.h"
#include "expression_analizer.h"

class CommandExecutor {
private:
	LexicalAnalyzer* lexicalAnalyzer;
	ExpressionAnalizer* expressionAnalizer;

	void executePrint() {}
	void executeGoTo() {}
	void executeIf() {}
	void executeFor() {}
	void executeNext() {}
	void executeInput() {}
	void executeGosub() {}
	void executeReturn() {}
	void executeEnd() {
		exit(1);
	}

public:
	CommandExecutor(LexicalAnalyzer* lexicalAnalyzer, ExpressionAnalizer* expressionAnalizer) {
		this->lexicalAnalyzer = lexicalAnalyzer;
		this->expressionAnalizer = expressionAnalizer;
	}

	void executeCommand(Token token) {

	}
};