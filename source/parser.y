%{
#include "parser.tab.h"
#include "error.h"

#define YYDEBUG 1
%}
%define parse.error verbose
%locations

%union {
    TreeNode* node;
}

%token <node> SUM SUB MUL DIV SLASH PERCENT SET NOTEQUAL EQUALITY
%token <node> LESSTHAN GREATERTHAN LESSTHANEQ GREATERTHANEQ
%token <node> AND OR NOT
%token <node> DECREMENT INCREMENT
%token <node> FUNCTION
%token <node> AS
%token <node> ARRAY
%token <node> DEF END BEGIN_BLOCK
%token <node> WEND
%token <node> IDENTIFIER
%token <node> STR
%token <node> COMMA
%token <node> CHAR
%token <node> BIN HEX DEC BOOL
%token <node> IF ELSE WHILE UNTIL DO BREAK
%token <node> THEN
%token <node> LOOP
%token <node> SEMICOLON
%token <node> LPAREN RPAREN
%token <node> TYPEDEF
%token <node> DIM

%left SET

%left AND OR

%left EQUALITY NOTEQUAL

%left LESSTHAN GREATERTHAN LESSTHANEQ GREATERTHANEQ

%left SUM SUB

%left MUL DIV SLASH PERCENT

%left INCREMENT DECREMENT

%type <node> typeRef
%type <node> funcSignature
%type <node> argDef
%type <node> sourceItem
%type <node> listSourceItem
%type <node> statement
%type <node> var
%type <node> if
%type <node> break
%type <node> builtin
%type <node> custom
%type <node> array
%type <node> source
%type <node> listArgDef
%type <node> optionalTypeRef
%type <node> literal
%type <node> place
%type <node> expr
%type <node> listExpr
%type <node> callOrIndexer
%type <node> braces
%type <node> unary
%type <node> binary
%type <node> listVar
%type <node> else
%type <node> listStatement
%type <node> while
%type <node> do
%type <node> whileOrUntil
%type <node> arrayCommas
%type <node> funcDef

%%
/* SourceItem */

source: listSourceItem      {{TreeNode* elements[] = {$1};$$ = createNode("source", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

sourceItem: funcDef         {{TreeNode* elements[] = {$1};$$ = createNode("sourceItem", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

listSourceItem:             {{$$ = NULL;}}
    | sourceItem listSourceItem     {{TreeNode* elements[] = {$1, $2}; $$ = createNode("listSourceItem", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};



/* FuncSignature */

funcDef: FUNCTION funcSignature listStatement END FUNCTION {{TreeNode* elements[] = {$2, $3}; $$ = createNode("funcDef", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

funcSignature: IDENTIFIER LPAREN listArgDef RPAREN optionalTypeRef {{TreeNode* elements[] = {$3, $5};$$ = createNode("funcSignature", mallocChildNodes(*(&elements + 1) - elements, elements), $1->value);}};

listArgDef:                 {{$$ = NULL;}}
    | argDef listArgDef     {{TreeNode* elements[] = {$1, $2};$$ = createNode("listArgDef", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}; //чтобы не было listArgDef с двумя argDef
    | argDef COMMA listArgDef       {{TreeNode* elements[] = {$1, $3};$$ = createNode("listArgDef", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

argDef: IDENTIFIER optionalTypeRef  {{TreeNode* elements[] = {$1, $2};$$ = createNode("argDef", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

optionalTypeRef:            {{ $$ = NULL; }}
    | AS typeRef            {{$$ = $2;}};



/* TypeRef */

typeRef: builtin            {{$$ = $1;}}
    | custom                {{$$ = $1;}}
    | array                 {{$$ = $1;}};

builtin: TYPEDEF            {{$$ = $1;}};



/* Statement */

statement: var              {{$$ =  $1;}}
    | if                    {{$$ =  $1;}}
    | while                 {{$$ =  $1;}}
    | do                    {{$$ =  $1;}}
    | break                 {{$$ =  $1;}}
    | expr SEMICOLON        {{$$ = $1;}};

listStatement:              {{$$ = NULL;}}
    | statement listStatement   {{TreeNode* elements[] = {$1, $2};$$ = createNode("listStatement", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};


custom: IDENTIFIER {{$$ = $1;}};

array: typeRef LPAREN arrayCommas RPAREN {{TreeNode* elements[] = {$1, $3};$$ = createNode("array", mallocChildNodes(*(&elements + 1) - elements, elements), $2->value);}};

arrayCommas:                {{$$ = NULL;}}
    | COMMA arrayCommas     {{TreeNode* elements[] = {$1, $2};$$ = createNode("arrayCommas", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};


/* IF ELSE */

if: IF expr THEN listStatement else END IF {{TreeNode* elements[] = {$2, $4, $5};$$ = createNode("if", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

else: ELSE listStatement    {{TreeNode* elements[] = {$2};$$ = createNode("else", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    |                       {{$$ = NULL;}};


while: WHILE expr listStatement WEND {{TreeNode* elements[] = {$2, $3};$$ = createNode("while", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}

do: DO listStatement LOOP whileOrUntil expr {{TreeNode* elements[] = {$2, $4, $5};$$ = createNode("do", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}

whileOrUntil: WHILE         {{$$ = $1;}}
    | UNTIL                 {{$$ = $1;}}

break: BREAK {{;$$ = createNode("break", NULL, "");}};

expr: unary                 {{$$ = $1;}}
    | binary                {{$$ = $1;}}
    | braces                {{$$ = $1;}}
    | callOrIndexer         {{$$ = $1;}}
    | place                 {{$$ = $1;}}
    | literal               {{$$ = $1;}};

binary: expr SET expr     {{TreeNode* elements[] = {$1, $3};$$ = createNode("SET", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};
    | expr SUM expr        {{TreeNode* elements[] = {$1, $3};$$ = createNode("SUM", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr SUB expr       {{TreeNode* elements[] = {$1, $3};$$ = createNode("SUB", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr MUL expr        {{TreeNode* elements[] = {$1, $3};$$ = createNode("MUL", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr DIV expr        {{TreeNode* elements[] = {$1, $3};$$ = createNode("DIV", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr SLASH expr       {{TreeNode* elements[] = {$1, $3};$$ = createNode("SLASH", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr PERCENT expr     {{TreeNode* elements[] = {$1, $3};$$ = createNode("PERCENT", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr EQUALITY expr {{TreeNode* elements[] = {$1, $3};$$ = createNode("EQUALITY", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr NOTEQUAL expr    {{TreeNode* elements[] = {$1, $3};$$ = createNode("NOTEQUAL", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr LESSTHAN expr    {{TreeNode* elements[] = {$1, $3};$$ = createNode("LESSTHAN", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr GREATERTHAN expr {{TreeNode* elements[] = {$1, $3};$$ = createNode("GREATERTHAN", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr LESSTHANEQ expr  {{TreeNode* elements[] = {$1, $3};$$ = createNode("LESSTHANEQ", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr GREATERTHANEQ expr   {{TreeNode* elements[] = {$1, $3};$$ = createNode("GREATERTHANEQ", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr AND expr         {{TreeNode* elements[] = {$1, $3};$$ = createNode("AND", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr OR expr          {{TreeNode* elements[] = {$1, $3};$$ = createNode("OR", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

unary: INCREMENT expr            {{TreeNode* elements[] = {$2};$$ = createNode("INCREMENT", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | DECREMENT expr            {{TreeNode* elements[] = {$2};$$ = createNode("DECREMENT", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | NOT expr              {{TreeNode* elements[] = {$2};$$ = createNode("NOT", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

braces: LPAREN expr RPAREN  {{TreeNode* elements[] = {$2};$$ = createNode("braces", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

callOrIndexer: expr LPAREN listExpr RPAREN  {{TreeNode* elements[] = {$1, $3};$$ = createNode("callOrIndexer", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

listExpr:                   {{$$ = NULL;}}
    | expr listExpr         {{TreeNode* elements[] = {$1, $2};$$ = createNode("listExpr", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
    | expr COMMA listExpr   {{TreeNode* elements[] = {$1, $3};$$ = createNode("listExpr", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}

place: IDENTIFIER           {{$$ = $1;}};

literal: BOOL               {{$$ = $1;}}
    | STR                   {{$$ = $1;}}
    | CHAR                  {{$$ = $1;}}
    | HEX                   {{$$ = $1;}}
    | BIN                   {{$$ = $1;}}
    | DEC                   {{$$ = $1;}};



/* VAR */

listVar:  {{$$ = NULL;}}
    | IDENTIFIER listVar {{TreeNode* elements[] = {$1, $2};$$ = createNode("listVar", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}; //чтобы не было listVar с двумя identifier
    | IDENTIFIER COMMA listVar {{TreeNode* elements[] = {$1, $3};$$ = createNode("listVar", mallocChildNodes(*(&elements + 1) - elements, elements), "");}};

var: DIM listVar AS typeRef {{TreeNode* elements[] = {$2, $4};$$ = createNode("var", mallocChildNodes(*(&elements + 1) - elements, elements), "");}}
%%