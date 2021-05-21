#pragma once
#include "lexical_analyzer.h"
#include "expression_analizer.h"

class ForDepthLimitException : public exception {};
class GoSubDepthLimitException : public exception {};

struct ForCycle {
	Variable var; // Счетчик цикла
	int target; // Конечное значение
	char* position;
};

class ForStack {
private:
	vector<ForCycle>* stack = new vector<ForCycle>();

public:
	void push(ForCycle fc) {
		if (this->stack->size() >= 25) {
			throw ForDepthLimitException();
		}

		this->stack->push_back(fc);
	}

	bool isEmpty() {
		return this->stack->empty();
	}

	ForCycle pop() {
		ForCycle tmp = this->stack->back();
		this->stack->pop_back();
		return tmp;
	}

};

class GoSubStack {
private:
	vector<char*>* stack = new vector<char*>();

public:
	void push(char* fc) {
		if (this->stack->size() >= 25) {
			throw GoSubDepthLimitException();
		}

		this->stack->push_back(fc);
	}

	bool isEmpty() {
		return this->stack->empty();
	}

	char* pop() {
		char* tmp = this->stack->back();
		this->stack->pop_back();
		return tmp;
	}
};

class CommandExecutor {
private:
	LexicalAnalyzer* lexicalAnalyzer;
	ExpressionAnalizer* expressionAnalizer;
	VariablesStore* variablesStore;

	ForStack* forStack = new ForStack();
	GoSubStack* goSubStack = new GoSubStack();

	void executePrint() {
		double expResult;
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
			else { // Выражение
				this->lexicalAnalyzer->toPreviousToken();
				expResult = this->expressionAnalizer->calcNextExpression();
				token = this->lexicalAnalyzer->getToken();

				string str = to_string(expResult);
				cout << expResult;
				printableContentLenght += str.length();
			}

			if (token.outer == ";") {  // Вычисление числа пробелов при переходе к следующей табуляции
				spacesCount = 8 - (printableContentLenght % 8);
				printableContentLenght += spacesCount;
				while (spacesCount) {
					cout << ' ';
					spacesCount--;
				}
			}
			else if(token.outer == ",") {} // Ничего не делаем
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

	void executeGoTo() {
		Token labelNumberToken = this->lexicalAnalyzer->getToken();

		if (labelNumberToken.type != this->lexicalAnalyzer->tokenTypes.INTEGER) {
			this->lexicalAnalyzer->serror(0);
		}

		Label l = this->lexicalAnalyzer->getLabelByNumber(stoi(labelNumberToken.outer));
		lexicalAnalyzer->toLabel(l);
	}
	void executeIf() {
		int x, y, cond;
		char operation;

		double xExp = this->expressionAnalizer->calcNextExpression();

		Token token = this->lexicalAnalyzer->getToken();
		operation = token.outer[0];

		if (!strchr("=<>", operation)) {
			this->lexicalAnalyzer->serror(0);
		}
		double yExp = this->expressionAnalizer->calcNextExpression();

		cond = 0;
		switch (operation) {
		case '=':
			if (xExp == yExp) cond = 1;
			break;
		case '<':
			if (xExp < yExp) cond = 1;
			break;
		case '>':
			if (xExp > yExp) cond = 1;
			break;
		}

		if (cond) {
			token = this->lexicalAnalyzer->getToken();
			if (token.inner != this->lexicalAnalyzer->commandsInner.THEN) {
				this->lexicalAnalyzer->serror(8);
				return;
			}
			// Програма продолжается со след строчки => выполняется условие if
		}
		else this->lexicalAnalyzer->toNExtEOL();
	}
	void executeEnd() {
		exit(1);
	}
	void executeInput() {

		Token token = this->lexicalAnalyzer->getToken();
		if (token.type == lexicalAnalyzer->tokenTypes.QUOTE) {
			cout << token.outer;

			token = this->lexicalAnalyzer->getToken();
			
			if (token.outer != ",") this->lexicalAnalyzer->serror(1);
			token = this->lexicalAnalyzer->getToken();
		}
		else {
			cout << "? ";
		}
		Variable var;
		try {
			var = this->variablesStore->getVariableByName(token.outer);
		}
		catch (VariableNotFoundException) {
			var.name = token.outer;
			this->variablesStore->addVariable(var);
		}

		int value;
		cin >> value;

		var.value = value;
		this->variablesStore->setVariable(var);
	}
	void executeGosub() {
		Token token = this->lexicalAnalyzer->getToken();
		Label label;
		try {
			label = this->lexicalAnalyzer->getLabelByNumber(stoi(token.outer));
		}
		catch (LabelNotFoundException) {
			this->lexicalAnalyzer->serror(7);
		}
		this->goSubStack->push(this->lexicalAnalyzer->getProgramCursor());
		this->lexicalAnalyzer->setProgramCursor(label.programPosision);  // Место, куда переходим
	}
	void executeReturn() {
		if (!this->goSubStack->isEmpty()) { 
			this->lexicalAnalyzer->setProgramCursor(this->goSubStack->pop());
		}
		else {
			this->lexicalAnalyzer->serror(0);
		}
	}
	void executeFor() {
		ForCycle fc;

		// чтение управляющей переменной
		Token token = this->lexicalAnalyzer->getToken();
		this->executeAssigment(token);
		Variable var = this->variablesStore->getVariableByName(token.outer);
		fc.var = var;

		token = this->lexicalAnalyzer->getToken();
		if (token.inner != this->lexicalAnalyzer->commandsInner.TO) {
			this->lexicalAnalyzer->serror(9);
		} 

		double target = this->expressionAnalizer->calcNextExpression();
		fc.target = target;
		
		fc.position = this->lexicalAnalyzer->getProgramCursor();
		this->forStack->push(fc);
	}
	void executeNext() {

		if (this->forStack->isEmpty()) {
			this->lexicalAnalyzer->serror(10);
		}
		ForCycle fc = this->forStack->pop();

		double value = fc.var.value;
		value++;
		fc.var.value = value;
		this->variablesStore->setVariable(fc.var);

		if (value > fc.target) { return; }

		this->forStack->push(fc);

		this->lexicalAnalyzer->setProgramCursor(fc.position);
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
		else if (commandToken.inner == this->lexicalAnalyzer->commandsInner.END) {
			return this->executeEnd();
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

		double expResult = this->expressionAnalizer->calcNextExpression();
		var.value = expResult;
		this->variablesStore->setVariable(var);
	}
};