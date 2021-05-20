#include "lexical_analyzer.h"
#include "variables_manager.h"
#include "expression_analizer.h"
#include "command_executor.h"
#include <fstream>

void main() {
    setlocale(LC_ALL, "Russian");

    char* temp = (char*)malloc(1000);

    ifstream program;
    program.open("C:\\Users\\markp\\Desktop\\volsu\\6_semester\\processes\\basic_c\\proga1.txt");

    char* prog = temp;
    while (program.get(*temp)) {
        temp++;
    }
    *temp = '\0';

	LexicalAnalyzer* lexicalAnalyzer = new LexicalAnalyzer(prog);
    VariablesStore* variablesStore = new VariablesStore();
    ExpressionAnalizer* expressionAnalizer = new ExpressionAnalizer(lexicalAnalyzer, variablesStore);
    CommandExecutor* commandExecutor = new CommandExecutor(lexicalAnalyzer, expressionAnalizer, variablesStore);

    Token token;

    do {
        token = lexicalAnalyzer->getToken();

        if (token.type == lexicalAnalyzer->tokenTypes.VARIABLE) {
            commandExecutor->executeAssigment(token);
        }
        else if (token.type == lexicalAnalyzer->tokenTypes.COMMAND) {
            commandExecutor->executeCommand(token);
        }

    } while (token.inner != lexicalAnalyzer->commandsInner.FINISHED);
}

