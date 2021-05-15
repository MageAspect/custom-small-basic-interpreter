#include "lexical_analyzer.h"
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
    
    Token token;
    try {
        do {
            token = lexicalAnalyzer->getToken();
            cout << token.outer << endl;
        } while (token.inner != lexicalAnalyzer->FINISHED);
    }
    catch (TokenNotFoundException e) {
        cout << "Token not found";
    }
}

