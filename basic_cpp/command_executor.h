#pragma once
#include "lexical_analyzer.h"
#include "expression_analizer.h"
#include "boost\lexical_cast.hpp"

class CommandExecutor {
private:
	LexicalAnalyzer* lexicalAnalyzer;
	ExpressionAnalizer* expressionAnalizer;
	VariablesStore* variablesStore;



	void executePrint() {
		boost::variant<int, double> expResult;
		int printableContentLenght = 0; 
		int spacesCount;
		Token token;
		Token lastToken;

		do {
			token = this->lexicalAnalyzer->getToken();

			if (
				token.inner == this->lexicalAnalyzer->commandsInner.EOL
				|| token.inner == this->lexicalAnalyzer->commandsInner.FINISHED
				)
			{
				break;
			};

			if (token.type == this->lexicalAnalyzer->tokenTypes.QUOTE) {
				cout << token.outer;
				printableContentLenght += token.outer.length();
				
				token = this->lexicalAnalyzer->getToken();
			}
			else { // ¬ыражение
				this->lexicalAnalyzer->toPreviousToken();
				expResult = this->expressionAnalizer->calcNextExpersion();
				token = this->lexicalAnalyzer->getToken();

				string str = boost::lexical_cast<string>(expResult);
				cout << expResult;
				printableContentLenght += str.length();
			}

			if (token.outer == ";") {  //вычисление числа пробелов при переходе к следующей табул€ции
				spacesCount = 8 - (printableContentLenght % 8);
				printableContentLenght += spacesCount;
				while (spacesCount) {
					cout << ' ';
					spacesCount--;
				}
			}
			else if(token.outer == ",") {} // Ќичего не делаем
			else if (
				token.inner != this->lexicalAnalyzer->commandsInner.EOL
				&& token.inner != this->lexicalAnalyzer->commandsInner.FINISHED
				)
			{
				this->lexicalAnalyzer->serror(0);
			}

			lastToken = token;
		} while (token.outer == ";" || token.outer == ",");

		if (
			token.inner == this->lexicalAnalyzer->commandsInner.EOL 
			|| token.inner == this->lexicalAnalyzer->commandsInner.FINISHED
			) 
		{
			if (lastToken.outer != ";" && lastToken.outer != ",")
				cout << endl;
			else lexicalAnalyzer->serror(0);
		}
	}

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
	CommandExecutor(LexicalAnalyzer* lexicalAnalyzer, ExpressionAnalizer* expressionAnalizer, VariablesStore* variablesStore) {
		this->lexicalAnalyzer = lexicalAnalyzer;
		this->expressionAnalizer = expressionAnalizer;
		this->variablesStore = variablesStore;
	}

	void executeCommand(Token commandToken) {
		
		if (commandToken.inner == this->lexicalAnalyzer->commandsInner.PRINT) {
			return this->executePrint();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.GOTO) {
			return this->executeGoTo();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.IF) {
			return this->executeIf();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.FOR) {
			return this->executeFor();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.NEXT) {
			return this->executeNext();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.INPUT) {
			return this->executeInput();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.GOSUB) {
			return this->executeGosub();
		}
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.RETURN) {
			return this->executeReturn();
		}
	}

	void executeAssigment(Token variableToken) {

		if (variableToken.type != this->lexicalAnalyzer->tokenTypes.VARIABLE) {
			this->lexicalAnalyzer->serror(3);
		}

		Variable var;
		try {
			var = this->variablesStore->getVariableByName(variableToken.outer);
		}
		catch (VariableNotFoundException) {
			var.name = variableToken.outer;
			this->variablesStore->addVariable(var);
		}

		Token assigmentToken = this->lexicalAnalyzer->getToken();

		if (assigmentToken.outer != "=") {
			this->lexicalAnalyzer->serror(4);
		}

		boost::variant<int, double> expResult = this->expressionAnalizer->calcNextExpersion();
		var.value = expResult;
		this->variablesStore->setVariable(var);
	}
};