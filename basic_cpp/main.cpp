#include "lexical_analyzer.h"
#include "variables_manager.h"
#include <fstream>

void main() {
    char* prog = (char*)malloc(1000);

    ifstream program;
    program.open("C:\\Users\\markp\\Desktop\\volsu\\6_semester\\processes\\basic_c\\proga1.txt");

    char* temp = prog;
    while (program.get(*prog)) {
        prog++;
    }
    *prog = '\0';


	LexicalAnalyzer *lexicalAnalyzer = new LexicalAnalyzer(temp);
    VariablesStore* variablesStore = new VariablesStore();
    
    Token token;
    try {
        do {
            token = lexicalAnalyzer->getToken();
            if (token.type == lexicalAnalyzer->VARIABLE) {
            
            }
            else if (token.type == lexicalAnalyzer->COMMAND) {
                
            }

        } while (token.inner != lexicalAnalyzer->FINISHED);
    }
    catch (TokenNotFoundException e) {
        cout << "Token not found";
    }

    try {
        Variable var;
        var.name = "hello";
        var.value = 123;

        variablesStore->addVariable(var);
        cout << variablesStore->getVariableByName(var.name).name << endl;

        var.value = 123.5;
        variablesStore->setVariable(var);
        cout << variablesStore->getVariableByName(var.name).value << endl;

        
    }
    catch (VariableNotFoundException e) {
        cout << "Variable not found";
    }
    catch (VariableWrongNameException) {
        cout << "Variable Wrong name";
    }
    catch (VariableDublicateException) {
        cout << "Variable Dublicate !";
    }
}

