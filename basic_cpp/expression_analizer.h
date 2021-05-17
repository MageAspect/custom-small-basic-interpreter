#pragma once
#include "lexical_analyzer.h"
#include "variables_manager.h"
#include "boost\variant.hpp"
#include "cmath"
#include <typeinfo>

class ExpressionAnalizer {
private:
    Token currentToken;
	LexicalAnalyzer* lexicalAnalyzer;
	VariablesStore* variablesStore;

    void loadNextToken() {
        this->currentToken = this->lexicalAnalyzer->getToken();
    }

    template <typename T>
    void sum2(T* result) {
        char operation;
        T hold;

        this->mult3(result);
        while ((operation = this->currentToken.outer[0]) == '+' || operation == '-') {
            this->loadNextToken();
            this->mult3(&hold);
            this->arith(operation, result, &hold);
        }
    }

    template <typename T>
    void mult3(T* result) {
        char operation;
        T hold;

        this->deg4(result);

        while ((operation = this->currentToken.outer[0]) == '*' || operation == '/' || operation == '%') {
            this->loadNextToken();
            this->deg4(&hold);
            this->arith(operation, result, &hold);
        }
    }

    template <typename T>
    void deg4(T* result) {
        T hold;

        this->unar5(result);
        if (this->currentToken.outer[0] == '^') {
            this->loadNextToken();
            this->unar5(&hold);
            this->arith('^', result, &hold);
        }
    }

    template <typename T>
    void unar5(T* result) {
        char operation;

        operation = 0;
        if (
            this->currentToken.type == this->lexicalAnalyzer->tokenTypes.DELIMITER
            && (
                this->currentToken.outer[0] == '+' 
                || this->currentToken.outer[0] == '-'
                )
            ) 
        {
            operation = this->currentToken.outer[0];
            this->loadNextToken();
        }

        this->expInPar6(result);
        if (operation) unary(operation, result);
    }

    template <typename T>
    void expInPar6(T* result) {
        if (this->currentToken.outer[0] == '(' && this->currentToken.type == this->lexicalAnalyzer->tokenTypes.DELIMITER) {
            this->loadNextToken();
            this->sum2(result);
            if (this->currentToken.outer[0] != ')') lexicalAnalyzer->serror(1);
            this->loadNextToken();
        }
        else this->primitive(result);
    }

    template <typename T>
    void primitive(T* result) {
        if (this->currentToken.type == this->lexicalAnalyzer->tokenTypes.VARIABLE) {
            string varType = this->variablesStore->getVariableByName(this->currentToken.outer).value.type().name();

            if (varType == "int") {
                *result = boost::get<int>(this->variablesStore->getVariableByName(this->currentToken.outer).value);
            }
            if (varType == "double") {
                *result = boost::get<double>(this->variablesStore->getVariableByName(this->currentToken.outer).value);
            }

            this->loadNextToken();
            return;
        }
        else if (this->currentToken.type == this->lexicalAnalyzer->tokenTypes.INTEGER) {
            *result = stoi(this->currentToken.outer);
            this->loadNextToken();
            return;
        }
        else if (this->currentToken.type == this->lexicalAnalyzer->tokenTypes.DOUBLE) {
            *result = stod(this->currentToken.outer);
            this->loadNextToken();
            return;
        }
        else {
            lexicalAnalyzer->serror(0);
        }
    }

    template <typename T>
    void unary(char op, T* result) {
        if (op == '-') * result = -*result;
    }

    template <typename T1, typename T2>
    void arith(char operation, T1* res, T2* hol) {
        register int tmp, exp;

        switch (operation) {
        case '-':
            *res = *res - *hol;
            break;
        case '+':
            *res = *res + *hol;
            break;
        case '*':
            *res = *res * *hol;
            break;
        case '/':
            *res = (*res) / (*hol);
            break;
        case '%':
            *res = (int)*res % (int)*hol;
            break;
        case '^':
            *res = pow(*res, *hol);
            break;
        }
    } 

public:
	ExpressionAnalizer(LexicalAnalyzer* lexicalAnalyzer, VariablesStore* variablesStore) {
		this->lexicalAnalyzer = lexicalAnalyzer;
		this->variablesStore = variablesStore;
	}

    boost::variant<int, double> calcNextExpersion() {
        int expType = this->getNextExpresionType();
        boost::variant<int, double> result;
        
        if (expType == this->lexicalAnalyzer->tokenTypes.DOUBLE) {
            double* tempRes = new double();
            this->loadNextToken();

            this->sum2(tempRes);

            this->lexicalAnalyzer->toPreviousToken();

            return result = *tempRes;
        }
        
        if (expType == this->lexicalAnalyzer->tokenTypes.INTEGER) {
            int* tempRes = new int();
            this->loadNextToken();

            this->sum2(tempRes);

            this->lexicalAnalyzer->toPreviousToken();

            return result = *tempRes;
        }
	}

    int getNextExpresionType() {
        this->lexicalAnalyzer->saveProgramCursorPosition();
        int expType = 0;

        do {
            this->loadNextToken();
            if (this->currentToken.type == this->lexicalAnalyzer->tokenTypes.DOUBLE) {
                expType = this->lexicalAnalyzer->tokenTypes.DOUBLE;
            }
            if (this->currentToken.type == this->lexicalAnalyzer->tokenTypes.VARIABLE) {
                Variable var = variablesStore->getVariableByName(this->currentToken.outer);
                string varType = var.value.type().name();
                if (varType == "double") {
                    expType = this->lexicalAnalyzer->tokenTypes.DOUBLE;
                }
            }
        } while (this->currentToken.inner != this->lexicalAnalyzer->commandsInner.FINISHED);
        
        if (expType == 0) {
            expType = this->lexicalAnalyzer->tokenTypes.INTEGER;
        }
        
        this->lexicalAnalyzer->rollBackToSavedPosition();
        return expType;
    }
};