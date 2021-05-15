#include <ctype.h>
#include <string.h>
#include <string>
#include <iostream>
#include "setjmp.h"
#include "math.h"
#include "stdlib.h"

#define  DELIMITER 1
#define  VARIABLE  2
#define  NUMBER    3
#define  COMMAND   4
#define  STRING    5
#define  QUOTE     6
#define  EOL       9
#define  FINISHED  10

int get_token();     /* �������� ������� */
void putback();     /*  ���������� ������� ������� �� ������� ����� */
int iswhite(char c);/* ���������� "������", ���� "c" ������ ��� ��������� */
int isdelim(char c); /* ���������� "������", ���� "c" ����������� */
int look_up(char* s);/* �������������� ������ ������� �� ������� ������ �� ���������� �������������. */
void serror(int error);/*���������� �� ������*/

void get_exp(double* result);
void level2(double* result), level3(double* result), level4(double* result), level5(double* result), level6(double* result);
void primitive(double* result), arith(char o, double* r, double* h), unary(char o, double* r);
int find_var(char* s);


extern jmp_buf e_buf; /* ����� ����� ������� longjmp() */
extern int variables[26]; /* ���������� */
struct commands {
    char command[20];
    char tok;
};
extern commands table[];
extern char* prog;  /* ����� �������������� ��������� */
extern char token[80]; /* ������� ������������� ������� */
extern int token_type; /* ��� ������� */
extern int tok; /* ���������� ������������� ������� */

int get_token()
{
    char* temp;
    token_type = 0;
    tok = 0;
    temp = token;
    while (iswhite(*prog)) ++prog;  /* ������� �������� */

    if (*prog == '\0') /* ����� ����� */
    {
        *token = '\0';
        tok = FINISHED;
        return(token_type = DELIMITER);
    }

    if (*prog == '\r')
    {
        ++prog; ++prog;
        tok = EOL;
        *token = '\r';
        token[1] = '\n';
        token[2] = '\0';
        return (token_type = DELIMITER);
    }

    if (strchr("+-*^/%=;(),><", *prog)) /* ����������� */
    {
        *temp = *prog;
        prog++; /* ������� �� ��e������ ������� */
        temp++;
        *temp = '\0';
        return (token_type = DELIMITER);
    }

    if (*prog == '"') /* ������ � �������� */
    {
        prog++;
        while (*prog != '"' && *prog != '\r')
            *temp++ = *prog++;
        if (*prog == '\r')
            serror(1);
        prog++; *temp = '\0';
        return(token_type = QUOTE);
    }

    if (isdigit(*prog)) /* ����� */
    {
        while (!isdelim(*prog))
            *temp++ = *prog++;
        //  if ((*prog)=='.')               *temp++ = *prog++;
            //while (!isdelim(*prog))
            //*temp++ = *prog++;
        *temp = '\0';
        return(token_type = NUMBER);
    }

    if (isalpha(*prog)) /* ���������� ��� ������� */
    {
        while (!isdelim(*prog))
            *temp++ = *prog++;
        token_type = STRING;
    }
    *temp = '\0';
    /* ���������������, ���� ������ ���� ������� ��� ���������� */
    if (token_type == STRING)
    {
        tok = look_up(token);
        if (!tok)
            token_type = VARIABLE;
        else
            token_type = COMMAND; /* ��� ������� */
    }
    return token_type;
}

void putback()
{
    char* t;
    t = token;
    for (; *t; t++) prog--;
}

int iswhite(char c)
{
    if (c == ' ' || c == '\t') return 1;
    else return 0;
}

int isdelim(char c)
{
    if (strchr(" ;,+-<>/*%^=()", c) || c == '\t' || c == '\r' || c == 0)
        return 1;
    return 0;
}

int look_up(char* s)
{
    int i;
    char* p;
    /* �������������� � ������� ������� �������� */
    p = s;
    while (*p)
    {
        *p = tolower(*p); p++;
    }//tolower-����������� ��������� ����� � ��������

     /* ���� ������� ���������� � ������� */
    for (i = 0; *table[i].command; i++)
        if (!strcmp(table[i].command, s))
            return table[i].tok;
    return 0; /* ������� �� ���������� */
}

/* ������ ��������� �� ������ */
void serror(int error)
{
    static std::string e[] = {
        "�������������� ������ ",
        "�������� ������� ������ ",
        "��� �� ��������� ",
        "�������������� ������ ��������� ",
        "�� ���������� ",
        "������� ����� ����������� ",
        "������������ ����� ",
        "�������������� ����� ",
        "��������� �������� THEN ",
        "��������� �������� TO ",
        "������� ����������� ����� FOR ������� ����� ",
        "NEXT �� ������������� FOR ",
        "������� ����������� GOSUB ������� ����� ",
        "RETURN �� ������������� GOSUB "
        "��� ������"// 14 ������� �������
    };
    std::cout << e[error];
    longjmp(e_buf, 1); /* ������� � ����� ���������� */
}

void get_exp(double* result)
{
    get_token();
    if (!*token) {
        serror(2);
        return;
    }
    level2(result);
    putback(); /* ���������� ��������� �������� ������� ������� �� ������� ����� */
}

/* �������� ��� ��������� ���� ������ */
void level2(double* result)
{
    char op;
    double hold;

    level3(result);
    while ((op = *token) == '+' || op == '-') {
        get_token();
        level3(&hold);
        arith(op, result, &hold);
    }
}

/* ���������� ������������ ��� �������� ���� �������� */
void level3(double* result)
{
    char op;
    double hold;

    level4(result);

    while ((op = *token) == '*' || op == '/' || op == '%') {
        get_token();
        level4(&hold);
        arith(op, result, &hold);
    }
}

/* ��������� ������� ����� (�������������) */
void level4(double* result)
{
    double hold;

    level5(result);
    if (*token == '^') {
        get_token();
        level4(&hold);
        arith('^', result, &hold);
    }
}

/* ������� + ��� - */
void level5(double* result)
{
    char op;

    op = 0;
    if ((token_type == DELIMITER) && (*token == '+' || *token == '-')) {
        op = *token;
        get_token();
    }
    level6(result);
    if (op)
        unary(op, result);
}

/* ��������� ��������� � ������� ������� */
void level6(double* result)
{
    if ((*token == '(') && (token_type == DELIMITER))
    {
        get_token();
        level2(result);
        if (*token != ')')
            serror(1);
        get_token();
    }
    else
        primitive(result);
}

/* ����������� �������� ���������� �� �� ����� */
void primitive(double* result)
{
    switch (token_type) {
    case VARIABLE:
        *result = find_var(token);
        get_token();
        return;
    case NUMBER:
        *result = atof(token);
        get_token();
        return;
    default:
        serror(0);
    }
}

/* ���������� ����������������� ���������� */
void arith(char o, double* r, double* h)
{
    register int t, ex;

    switch (o) {
    case '-':
        *r = *r - *h;
        break;
    case '+':
        *r = *r + *h;
        break;
    case '*':
        *r = *r * *h;
        break;
    case '/':
        *r = (*r) / (*h);
        break;
    case '%':
        t = (*r) / (*h);
        *r = *r - (t * (*h));
        break;
    case '^':
        ex = *r;
        if (*h == 0) {
            *r = 1;
            break;
        }
        for (t = *h - 1; t > 0; --t)
            *r *= ex;
        break;
    }
}

/* ��������� ����� */
void unary(char o, double* r)
{
    if (o == '-') *r = -(*r);
}

/* ����� �������� ���������� */
int find_var(char* s)
{
    if (!isalpha(*s))
    {
        serror(4); /* �� ���������� */
        return 0;
    }
    return variables[toupper(*token) - 'A'];
}