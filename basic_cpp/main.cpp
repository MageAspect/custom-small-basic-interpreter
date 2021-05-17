#include "lexical_analyzer.h"
#include "variables_manager.h"
#include "expression_analizer.h"
#include <fstream>
#include <typeinfo>

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
    ExpressionAnalizer* expressionAnalizer = new ExpressionAnalizer(lexicalAnalyzer, variablesStore);

    variablesStore->addVariable(Variable("X", 15.411));
    
    cout << lexicalAnalyzer->getToken().outer << endl;

    boost::variant<int, double> result = 1.0;

    result = expressionAnalizer->calcNextExpersion();

    cout << result << " " << result.type().name();

}

