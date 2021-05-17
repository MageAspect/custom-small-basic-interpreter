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
	// ��������� �� ������������ ������ ���������
	char* programCursor;

	char* lastTokenStartPosition;

	char* savedProgramCursorPosition;

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
	// ��������� ������������� ��� ������. ���� ������� ����������� � ������������
	struct CommandsInner {
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
	} commandsInner;

	// ��������� ���� �������
	struct TokenTypes {
		const int DELIMITER = 1;
		const int VARIABLE = 2;
		const int INTEGER = 3;
		const int DOUBLE = 4;
		const int COMMAND = 5;
		const int STRING = 6;
		const int QUOTE = 7;
	} tokenTypes;

	LexicalAnalyzer(char* programCode) {
		// ���������� ���������� ���������
		this->commands->push_back(Command("print", this->commandsInner.PRINT));
		this->commands->push_back(Command("input", this->commandsInner.INPUT));
		this->commands->push_back(Command("if", this->commandsInner.IF));
		this->commands->push_back(Command("then", this->commandsInner.GOTO));
		this->commands->push_back(Command("goto", this->commandsInner.FOR));
		this->commands->push_back(Command("for", this->commandsInner.NEXT));
		this->commands->push_back(Command("to", this->commandsInner.TO));
		this->commands->push_back(Command("gosub", this->commandsInner.GOSUB));
		this->commands->push_back(Command("return", this->commandsInner.RETURN));
		this->commands->push_back(Command("end", this->commandsInner.END));
		this->commands->push_back(Command("", this->commandsInner.END));

		this->programCursor = programCode;
		this->savedProgramCursorPosition = programCode;
		this->lastTokenStartPosition = programCode;
	}

	Token getToken() {
		Token* token = new Token();

		while (isspace(*this->programCursor)) {
			this->programCursor++;
		}

		this->lastTokenStartPosition = this->programCursor;

		if (*this->programCursor == 0) { // '\0'
			token->outer = "";
			token->inner = this->commandsInner.FINISHED;
			token->type = this->tokenTypes.DELIMITER;

			return *token;
		}

		if (*this->programCursor == 10) { // '\r'
			token->inner = this->commandsInner.EOL;
			token->outer = "\r";
			token->type = this->tokenTypes.DELIMITER;

			this->programCursor++;

			return *token;
		}

		if (strchr("+-*^/%=;(),><", *this->programCursor)) {
			token->outer = *this->programCursor;
			token->type = this->tokenTypes.DELIMITER;

			this->programCursor++;

			return *token;
		}

		if (*this->programCursor == '"') {
			this->programCursor++;

			while (*this->programCursor != '"' && *this->programCursor != 10) {
				token->outer += *this->programCursor++; // ����������� ������ ��� ������ ���������� ����������
			}

			// ���� ��������� ������� ������ �� ����, ��� ���� ��������� ����������� �������
			if (*this->programCursor == 10) this->serror(1);// \n
			this->programCursor++;

			token->type = this->tokenTypes.QUOTE;

			return *token;
		}

		if (isdigit(*this->programCursor)) {
			token->type = this->tokenTypes.INTEGER;

			while (!this->isDelimiter(*this->programCursor)) {
				if (*this->programCursor == '.') {
					token->type = this->tokenTypes.DOUBLE;
				}
				token->outer += *this->programCursor++;
			}

			return *token;
		}

		if (isalpha(*this->programCursor)) {

			while (!this->isDelimiter(*this->programCursor)) {
				token->outer += *this->programCursor++;
			}
			token->type = this->tokenTypes.STRING;
		}

		if (token->type == this->tokenTypes.STRING) {
			try {
				Command command = this->getCommandByOuter(token->outer);
				token->type = this->tokenTypes.COMMAND;
				token->inner = command.inner;
			}
			catch (exception &e) {
				token->type = this->tokenTypes.VARIABLE;
			}
			return *token;
		}

		this->lastTokenStartPosition = nullptr;
		throw TokenNotFoundException();
	}

	// ��������� ������� ��������� �� ������������� ������ ��������� this->programCursor
	void saveProgramCursorPosition() {
		this->savedProgramCursorPosition = this->programCursor;
	}

	// ���������� ������� ��������� this->programCursor �� ������ ������ saveProgramCursorPosition
	void rollBackToSavedPosition() {
		if (this->savedProgramCursorPosition) {
			this->programCursor = this->savedProgramCursorPosition;
		}
	}

	// ���������� programCursor �� ���� ����� �����
	void toPreviousToken() {
		if (this->lastTokenStartPosition != nullptr) {
			this->programCursor = this->lastTokenStartPosition;
		}
	}

	void toNextToken() {
		if (this->lastTokenStartPosition != nullptr) {
			this->getToken();
		}
	}

	void serror(int type) {
		cout << "gg" << endl;
		exit(1);
	}

};


