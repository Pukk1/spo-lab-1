%{
#include "error.h"
#include "parser.tab.h"
#include <string.h>
%}

%option yylineno

%%

[0-9]+ {
    yylval.node = createNode("DEC", NULL, yytext);
    return DEC;
}
\"[^\"\\]*(?:\\.[^\"\\]*)*\" {
    char buffer[1024];
    strncpy(buffer, yytext+1, strlen(yytext)-1);
    buffer[strlen(buffer)-1] = 0;
    yylval.node = createNode("STR", NULL, buffer);
    return STR;
}
'[^']' {
    char buffer[1];
    strncpy(buffer, yytext+1, 1);
    yylval.node = createNode("CHAR", NULL, buffer);
    return CHAR;
}

0[xX][0-9A-Fa-f]+ {
    char* buffer = malloc(256);
    sprintf(buffer, "%ld", strtol(yytext + 2, NULL, 16));
    yylval.node = createNode("HEX", NULL, buffer);
    return HEX;
}
0[bB][01]+ {
    char* buffer = malloc(256);
    sprintf(buffer, "%ld", strtol(yytext + 2, NULL, 2));
    yylval.node = createNode("BIN", NULL, buffer);
    return BIN;
}
"byte" {
           yylval.node = createNode("TYPEDEF", NULL, yytext);
           return TYPEDEF;
       }
"int" {
          yylval.node = createNode("TYPEDEF", NULL, yytext);
          return TYPEDEF;
      }
"uint" {
           yylval.node = createNode("TYPEDEF", NULL, yytext);
           return TYPEDEF;
       }
"long" {
           yylval.node = createNode("TYPEDEF", NULL, yytext);
           return TYPEDEF;
       }
"bool" {
           yylval.node = createNode("TYPEDEF", NULL, yytext);
           return TYPEDEF;
       }
"ulong" {
            yylval.node = createNode("TYPEDEF", NULL, yytext);
            return TYPEDEF;
        }
"char" {
           yylval.node = createNode("TYPEDEF", NULL, yytext);
           return TYPEDEF;
       }
"string"  {
              yylval.node = createNode("TYPEDEF", NULL, yytext);
              return TYPEDEF;
          }
"dim"                                {
 yylval.node = createNode("DIM", NULL, "");
 return DIM;
 }
"if"                                {
 yylval.node = createNode("IF", NULL, "");
 return IF;
 }
"as"                                {
 yylval.node = createNode("AS", NULL, "");
 return AS;
 }
"function"                                {
 yylval.node = createNode("FUNCTION", NULL, "");
 return FUNCTION;
 }
"end"                                {
 yylval.node = createNode("END", NULL, "");
 return END;
 }
"else"                              {
 yylval.node = createNode("ELSE", NULL, "");
 return ELSE;
 }
"then"                              {
 yylval.node = createNode("THEN", NULL, "");
 return THEN;
 }
"do"                                {
 yylval.node = createNode("DO", NULL, "");
 return DO;
 }
"loop"                                {
 yylval.node = createNode("LOOP", NULL, "");
 return LOOP;
 }
"while"                             {
 yylval.node = createNode("WHILE", NULL, "");
 return WHILE;
 }
"wend"                             {
 yylval.node = createNode("WEND", NULL, "");
 return WEND;
 }
"until"                             {
 yylval.node = createNode("UNTIL", NULL, "");
 return UNTIL;
 }
"break"                             {
 yylval.node = createNode("BREAK", NULL, "");
 return BREAK;
 }
"true"                              {
 yylval.node = createNode("BOOL", NULL, yytext);
 return BOOL;
 }
"false"                             {
 yylval.node = createNode("BOOL", NULL, yytext);
 return BOOL;
 }
"%"                                 {
 return PERCENT;
 }
";"                                 {
 return SEMICOLON;
 }
","                                 {
 yylval.node = createNode("COMMA", NULL, "");
 return COMMA;
 }
"!="                                {
 return NOTEQUAL;
 }
"=="                                {
 return EQUALITY;
 }
"!"                                 {
 return NOT;
 }
"\|\|"                              {
 return OR;
 }
"&&"                                {
 return AND;
 }
"+"                                 {
 return SUM;
 }
"-"                                 {
 return SUB;
 }
"++"                                 {
 return INCREMENT;
 }
"--"                                 {
 return DECREMENT;
 }
"<="                                {
 return LESSTHANEQ;
 }
">="                                {
 return GREATERTHANEQ;
 }
"<"                                 {
 return LESSTHAN;
 }
">"                                 {
 return GREATERTHAN;
 }
"*"                                 {
 return MUL;
 }
"/"                                 {
 return DIV;
 }
"("                                 {
 return LPAREN;
 }
")"                                 {
 return RPAREN;
 }
"="                                 {
 return SET;
 }
[a-zA-Z_][a-zA-Z_0-9]* {
    yylval.node = createNode("IDENTIFIER", NULL, yytext);
    return IDENTIFIER;
}
\/\/.*                               { /* Пропустить комментарии */ }
\n                                  { /* Пропустить переводы строк */ }
[ \t]                               { /* Пропустить пробелы и табуляцию */ }
.                                   {
                                        // Обработка нераспознанных символов
                                        fprintf(stderr, "Нераспознанный символ: %s\n", yytext);
                                    }
%%


int yywrap() {
    return 1;
}