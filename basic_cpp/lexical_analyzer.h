#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

class CommandNotFoundException : public exception {};
class TokenNotFoundException : public exception {};

struct Token
{
	string outer;
	int inner;
	int type;
};

struct Command {
	Command(string outer, int inner) {
		this->outer = outer;
		this->inner = inner;
	}
	string outer;
	int inner;
};

class LexicalAnalyzer
{
private:
	// Указатель на анализируемы символ программы
	char* programCursor;

	vector<Command>* commands = new vector<Command>();

	Command getCommandByOuter(string outer) {
		for (Command command : *this->commands) {
			transform(
				outer.begin(),
				outer.end(),
				outer.begin(),
				::tolower
			);

			if (command.outer == outer) {
				return command;
			}
		}
		throw CommandNotFoundException();
	}

	bool isDelimiter(char simbol)
	{
		if (
			strchr(" !;,+-<>'/*%^=()", simbol)
			|| simbol == 9
			|| simbol == 10
			|| simbol == 0
			)
		{
			return true;
		}
		return false;
	}

public:

	LexicalAnalyzer(char* programCode) {
		// Заполнение доступными командами
		this->commands->push_back(Command("print", PRINT));
		this->commands->push_back(Command("input", INPUT));
		this->commands->push_back(Command("if", IF));
		this->commands->push_back(Command("then", GOTO));
		this->commands->push_back(Command("goto", FOR));
		this->commands->push_back(Command("for", NEXT));
		this->commands->push_back(Command("to", TO));
		this->commands->push_back(Command("gosub", GOSUB));
		this->commands->push_back(Command("return", RETURN));
		this->commands->push_back(Command("end", END));
		this->commands->push_back(Command("", END));

		this->programCursor = programCode;
	}

	// Типы токенов
	const int DELIMITER = 111;
	const int VARIABLE = 222;
	const int NUMBER = 333;
	const int COMMAND = 444;
	const int STRING = 555;
	const int QUOTE = 666;

	// Команды
	const int PRINT = 1;
	const int INPUT = 2;
	const int IF = 3;
	const int THEN = 4;
	const int FOR = 5;
	const int NEXT = 6;
	const int TO = 7;
	const int GOTO = 8;
	const int EOL = 9;
	const int FINISHED = 10;
	const int GOSUB = 11;
	const int RETURN = 12;
	const int END = 13;

	Token getToken() {
		Token* token = new Token();

		while (isspace(*this->programCursor))++this->programCursor;

		if (*this->programCursor == 0) { // '\0'
			token->outer = "";
			token->inner = FINISHED;
			token->type = DELIMITER;

			return *token;
		}

		if (*this->programCursor == 10) { // '\r'
			token->inner = EOL;
			token->outer = "\r";
			token->type = DELIMITER;

			this->programCursor++;

			return *token;
		}

		if (strchr("+-*^/%=;(),><", *this->programCursor)) {
			token->outer = *this->programCursor;
			token->type = DELIMITER;

			this->programCursor++;

			return *token;
		}

		if (*this->programCursor == '"') {
			this->programCursor++;

			while (*this->programCursor != '"' && *this->programCursor != 10) {
				token->outer += *this->programCursor++; // копирование строки при помощи арифметики указателей
			}

			if (*this->programCursor == 10) this->serror(1);// \n
			this->programCursor++;

			token->type = QUOTE;

			return *token;
		}

		if (isdigit(*this->programCursor)) {

			while (!this->isDelimiter(*this->programCursor)) {
				token->outer += *this->programCursor++;
			}
			
			token->type = NUMBER;

			return *token;
		}

		if (isalpha(*this->programCursor)) {

			while (!this->isDelimiter(*this->programCursor)) {
				token->outer += *this->programCursor++;
			}
			token->type = STRING;
		}

		if (token->type == STRING) {
			try {
				Command command = this->getCommandByOuter(token->outer);
				token->type = COMMAND;
				token->inner = command.inner;
			}
			catch (exception &e) {
				token->type = VARIABLE;
			}
			return *token;
		}

		throw TokenNotFoundException();
	}

	// Откатывает programCursor на переданный токен назад
	void putBack(Token token) {
		for (int i = 0; i < token.outer.length(); i++) {
			this->programCursor--;
		}
	}

	void serror(int type) {
		cout << "gg" << endl;
		exit(1);
	}

};


