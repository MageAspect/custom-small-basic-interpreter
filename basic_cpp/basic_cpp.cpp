/* Минимальный BASIC-интепретатор */

#include "stdio.h"
#include "setjmp.h"
#include "math.h"
#include "ctype.h"
#include "stdlib.h"
#include <string.h>
#include<iostream>
using namespace std;

#define NUM_LAB 100
#define LAB_LEN 10
#define FOR_NEST 25
#define SUB_NEST 25
#define PROG_SIZE 10000

#define DELIMITER 1
#define VARIABLE  2
#define NUMBER    3
#define COMMAND   4
#define STRING    5
#define QUOTE     6

#define PRINT 1
#define INPUT 2
#define IF    3
#define THEN  4
#define FOR   5
#define NEXT  6
#define TO    7
#define GOTO  8
#define EOL   9
#define FINISHED 10
#define GOSUB 11
#define RETURN 12
#define END   13


jmp_buf e_buf; /* содержит среду для longjmp() */
double variables[26] = {   /* 26 переменных пользователя A - Z */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};
int tok, token_type;
char* prog;  /* Содержит анализируемое выражение */
struct commands { /* Просмотр таблицы ключевых слов */
    char command[20];
    char tok;
} table[12] = { /* Команда должна вводится прописными */
    "print",PRINT, /* буквами в эту таблицу */
    "input",INPUT,
    "if",IF,
    "then",THEN,
    "goto",GOTO,
    "for",FOR,
    "next",NEXT,
    "to",TO,
    "gosub",GOSUB,
    "return",RETURN,
    "end",END,
    "",END  /* Маркер конца таблицы */
};
char token[80];

int load_program(char* p, char* fname);//Выполняет загрузку программы

void assignment();//функция присваивания
void print();//функция печати
void input();//функция считывания
void exec_if();//Реализует IF

void exec_goto();//Функция реализующая GOTO
/*---------------вспомогательные функции и данные для GOTO---------------*/
struct label {
    char name[LAB_LEN];
    char* p;
};
struct label label_table[NUM_LAB];
void   label_init();
void  scan_labels();
void find_eol();
int get_next_label(char* s);
char* find_label(char* s);
/*-----------------------------------------------------------------------*/

void exec_for();//функция реализующая цикл FOR
/*--------------вспомогательные функции и данные для FOR-----------------*/
struct for_stack {
    int var; /* переменная счетчика */
    double target; /* конечное значение */
    char* loc;
};
for_stack fstack[FOR_NEST]; /* стек цикла FOR/NEXT */
int ftos; /* индекс начала стека FOR */
for_stack fpop();
void fpush(struct for_stack i);
void next();
/*----------------------------------------------------------------------*/

void gosub();//Функция реализующая GOSUB
/*---------------------вспомогательные функции для GOSUB----------------*/
char* gstack[SUB_NEST]; /* стек оператора GOSUB */
int gtos; /* индекс начала стека GOSUB */
void gpush(char* s);
char* gpop();
void greturn();
//find_label(char *) объявлена выше в блоке GOTO
/*----------------------------------------------------------------------*/


/*--------------------Эти методы реализованы в файле LexicalAnalyzer.cpp--------------------------------------*/
extern void get_exp(double* result);
extern void level2(double* result), level3(double* result), level4(double* result), level5(double* result), level6(double* result);
extern void primitive(double* result), arith(char o, int* r, int* h), unary(char o, int* r);
extern int find_var(char* s);
extern int get_token();  /* Получить лексему */
extern void putback();      /*  Возвращает лексему обратно во входной поток */
extern int iswhite(char c);/* Возвращает "истину", если "c" пробел или табуляция */
extern int isdelim(char c); /* Возвращает "истину", если "c" разделитель */
extern int look_up(char* s);/* Преобразование каждой лексемы из таблицы лексем во внутреннее представление. */
extern void serror(int error);/*информация об ошибке*/
/*-----------------------------------------------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    char* p_buf;

    if (argc != 2) {
        std::cout << "Используйте формат: Small_Basic <filename>\n";
        exit(1);
    }

    /* Выделение памяти для программы */
    if (!(p_buf = (char*)malloc(PROG_SIZE))) {
        printf("Ошибка при выделении памяти ");
        exit(1);
    }

    /* Загрузка программы для выполнения */
    if (!load_program(p_buf, argv[1]))
        exit(1);
    if (setjmp(e_buf))
        exit(1); /* инициализация буфера
                                нелокальных переходов */
    prog = p_buf;
    scan_labels(); /* поиск метки в программе */
    ftos = 0; /* инициализация индеса стека FOR */
    gtos = 0; /* инициализация индеса стека GOSUB */
    do {
        token_type = get_token();
        /* проверка на оператор присваивания */
        if (token_type == VARIABLE) {
            putback(); /* возврат пер. обратно во входной поток */
            assignment(); /* должен быть оператор присваивания */
        }
        else /* это команда */
            switch (tok) {
            case PRINT:
                print();
                break;
            case INPUT:
                input();
                break;
            case IF:
                exec_if();
                break;
            case END:
                exit(0);
            case GOTO:
                exec_goto();
                break;
            case FOR:
                exec_for();
                break;
            case NEXT:
                next();
                break;
            case GOSUB:
                gosub();
                break;
            case RETURN:
                greturn();
                break;
            }
    } while (tok != FINISHED);
    return 0;
}

/* Загрузка программы. */
int load_program(char* p, char* fname)
{
    FILE* fp;
    int i = 0;
    if (!(fp = fopen_s(fname, "rb"))) return 0;
    do {
        *p = getc(fp);
        p++; i++;
    } while (!feof(fp) && i < PROG_SIZE);
    *(p - 1) = '\0'; // символ конца программы
    fclose(fp);
    return 1;
}

/* Присваивание переменной значения */
void assignment()
{
    int var;
    double value;
    /* Получить имя переменной */
    get_token();
    if (!isalpha(*token))
    {
        serror(4); /* это не переменная */
        return;
    }
    /* вычисление индекса переменной */
    var = toupper(*token) - 'A';

    /* получить знак равенства */
    get_token();
    if (*token != '=') {
        serror(3);
        return;
    }

    /* получить значение, присваемое переменной */
    get_exp(&value);

    /* присвоить это значение */
    variables[var] = value;
}

/* Простейшая реализация оператора PRINT */
void print()
{
    double answer;
    char last_delim;

    do {
        get_token(); /* получить следующий элемент списка  */
        if (tok == EOL || tok == FINISHED) break;
        if (token_type == QUOTE) { /* это строка */
            std::cout << token;
            get_token();
        }
        else { /* это выражение */
            putback();
            get_exp(&answer);
            get_token();
            std::cout << answer;
        }
        last_delim = *token;

        if (*token == ';')
            std::cout << '\t';
        else if (*token != ',' && tok != EOL && tok != FINISHED)  serror(0);
    } while (*token == ';' || *token == ',');

    if (tok == EOL || tok == FINISHED)
    {
        if (last_delim != ';' && last_delim != ',')
            std::cout << endl;
    }
    else serror(0); /* отсутствует ',' или ';'  */
}

/* Реализация оператора INPUT */
void input()
{
    int i, var;

    get_token(); /*просматривается если существует строка символов*/
    if (token_type == QUOTE) {
        std::cout << token; /* если да, то ее печать и контроль ',' */
        get_token();
        if (*token != ',') serror(1);
        get_token();
    }
    else std::cout << "? "; /* выдача строки по умолчанию */
    var = toupper(*token) - 'A'; /* получить индекс имени переменной*/
    std::cin >> i; /* чтение ввода данных */
    variables[var] = i;  /* сохранение данных */
}

/* Реализация оператора IF  */
void exec_if()
{
    int cond;
    double x, y;
    char op;
    get_exp(&x); /* получить левое выражение */
    get_token(); /* получить оператор */
    if (!strchr("=<>", *token)) {
        serror(0);      /* недопустимый оператор */
        return;
    }
    op = *token;
    get_exp(&y);  /* получить правое выражение */
                  /* Определение результата */
    cond = 0;
    switch (op) {
    case '=':
        if (x == y) cond = 1;
        break;
    case '<':
        if (x < y) cond = 1;
        break;
    case '>':
        if (x > y) cond = 1;
        break;
    }
    if (cond) {  /* если значение IF "истина"  */
        get_token();
        if (tok != THEN) {
            serror(8);
            return;
        } /* иначе, программа выполняется со следующей строки */
    }
    else find_eol(); /* поиск точки старта программы */
}

/* Реализация оператора GOTO */
void exec_goto()
{
    char* loc;

    get_token(); /* получить метку перехода */
                 /* Поиск местоположения метки */
    loc = find_label(token);
    if (loc == '\0')
        serror(7);   /* метка не обнаружена  */
    else prog = loc; /* старт программы с указанной точки  */
}

/* Инициализация массива хранения меток. По договоренности
нулевое значение метки символизирует пустую ячейку массива */
void label_init()
{
    for (int t = 0; t < NUM_LAB; t++)
        label_table[t].name[0] = '\0';
}

/* Поиск всех меток */
void scan_labels()
{
    int addr;
    char* temp;
    label_init();  /* обнуление всех меток */
    temp = prog; /* сохраним указатель на начало программы*/
                 /* Если первая лексема файла есть метка  */
    get_token();
    if (token_type == NUMBER) {
        strcpy(label_table[0].name, token);
        label_table[0].p = prog;
    }
    find_eol();
    do {
        get_token();
        if (token_type == NUMBER) {
            addr = get_next_label(token);
            if (addr == -1)
                serror(5);
            else if (addr == -2)
                serror(6);
            else
            {
                strcpy(label_table[addr].name, token);
                label_table[addr].p = prog; /* текущий указатель программы */
            }
        }

        if (tok != EOL)//переходим на следующую строку
            find_eol();
    } while (tok != FINISHED);
    prog = temp; /* сохраним оригинал */
}

/* Поиск начала следующей строки */
void find_eol()
{
    while (*prog != '\n' && *prog != '\0')
        ++prog;
    if (*prog) prog++;
}

/* Возвращает индекс ена следующую свободную позицию
массива меток. -1, если массив переполнен.
-2, если дублирование меток. */
int get_next_label(char* s)
{
    for (int t = 0; t < NUM_LAB; ++t) {
        if (label_table[t].name[0] == 0)
            return t;
        if (!strcmp(label_table[t].name, s))
            return -2; /*дублирование*/
    }
    return -1;//нет свободных мест
}

/* Поиск строки по известной метке. Значение 0 возвращается,
если метка не найдена; в противном случае возвращается
указатель на помеченную строку программы        */
char* find_label(char* s)
{
    int t;
    for (t = 0; t < NUM_LAB; ++t)
        if (!strcmp(label_table[t].name, s))
            return label_table[t].p;
    return '\0'; /* состояние ошибки */
}


/* Реализация цикла FOR */
void exec_for()
{
    for_stack i;
    double value;

    get_token(); /* получить управляющую переменную */
    if (!isalpha(*token))
    {
        serror(4);
        return;
    }
    i.var = toupper(*token) - 'A'; /* сохранить ее индекс */
    get_token(); /* получить знак равенства */
    if (*token != '=') {
        serror(3);
        return;
    }
    get_exp(&value); /* получить начальное значение  */
    variables[i.var] = value;
    get_token();
    if (tok != TO) serror(9); /* если не нашли то ошибка */
    get_exp(&i.target); /* получить конечное значение */
                        /* Если цикл выполнится хотя бы раз, поместить
                        информацию в стек  */
    if (i.target >= variables[i.var]) {
        i.loc = prog;
        fpush(i);
    }
    else /* иначе пропустить весь цикл */
        while (tok != NEXT)
            get_token();
}

/* Поместить информацию в стек FOR */
void fpush(struct for_stack i)
{
    if (ftos > FOR_NEST)
        serror(10);
    fstack[ftos] = i;
    ftos++;
}

for_stack fpop()
{
    ftos--;
    if (ftos < 0) serror(11);
    return(fstack[ftos]);
}

/* Реализация оператора NEXT */
void next()
{
    for_stack i = fpop(); /* чтение информации о цикле */
    variables[i.var]++; /* увеличение управляющей переменной*/
    if (variables[i.var] > i.target)
        return; /* конец цикла */
    fpush(i); /* иначе, сохранить информацию в стеке */
    prog = i.loc; /* цикл */
}


/* Реализация оператора GOSUB */
void gosub()
{
    get_token();
    /* поиск метки вызова */
    char* loc = find_label(token);
    if (loc == '\0')
        serror(7); /* метка не определена */
    else {
        gpush(prog); /* запомним место, куда вернемся */
        prog = loc; /* старт программы с указанной точки */
    }
}

/* Помещает данные в стек GOSUB */
void gpush(char* s)
{
    gtos++;
    if (gtos == SUB_NEST) {
        serror(12);
        return;
    }
    gstack[gtos] = s;
}

/*Извлекает данные из стека GOSUB*/
char* gpop()
{
    if (gtos == 0) {
        serror(13);
        return 0;
    }
    return(gstack[gtos--]);
}

/* Возврат из подпрограммы, вызвынной по GOSUB */
void greturn()
{
    prog = gpop();
}