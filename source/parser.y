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

%token <node> PLUS MINUS MUL SLASH PERCENT EQUAL NOTEQUAL
%token <node> LESSTHAN GREATERTHAN LESSTHANEQ GREATERTHANEQ
%token <node> AND OR NOT
%token <node> FUNCTION
%token <node> AS
%token <node> ARRAY
%token <node> DEF END BEGIN_BLOCK
%token <node> WEND
%token <node> IDENTIFIER
%token <node> STR
%token <node> COMMA
%token <node> CHAR
%token <node> BIN HEX DEC
%token <node> TRUE FALSE
%token <node> IF ELSE WHILE UNTIL DO BREAK
%token <node> THEN
%token <node> LOOP
%token <node> SEMICOLON
%token <node> LPAREN RPAREN
%token <node> TYPEDEF
%token <node> DIM


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

source: listSourceItem      {{TreeNode* nodes[] = {$1};$$ = createNode("source", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

sourceItem: funcDef         {{TreeNode* nodes[] = {$1};$$ = createNode("sourceItem", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

listSourceItem:             {{$$ = NULL;}}
    | sourceItem listSourceItem     {{TreeNode* nodes[] = {$1, $2}; $$ = createNode("listSourceItem", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};



/* FuncSignature */

funcDef: FUNCTION funcSignature listStatement END FUNCTION {{TreeNode* nodes[] = {$2, $3}; $$ = createNode("funcDef", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

funcSignature: IDENTIFIER LPAREN listArgDef RPAREN optionalTypeRef {{TreeNode* nodes[] = {$3, $5};$$ = createNode("funcSignature", mallocChildNodes(*(&nodes + 1) - nodes, nodes), $1->value);}};

listArgDef:                 {{$$ = NULL;}}
    | argDef listArgDef     {{TreeNode* nodes[] = {$1, $2};$$ = createNode("listArgDef", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}; //чтобы не было listArgDef с двумя argDef
    | argDef COMMA listArgDef       {{TreeNode* nodes[] = {$1, $3};$$ = createNode("listArgDef", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

argDef: IDENTIFIER optionalTypeRef  {{TreeNode* nodes[] = {$1, $2};$$ = createNode("argDef", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

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
    | statement listStatement   {{TreeNode* nodes[] = {$1, $2};$$ = createNode("listStatement", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};


custom: IDENTIFIER {{$$ = $1;}};

array: typeRef LPAREN arrayCommas RPAREN {{TreeNode* nodes[] = {$1, $3};$$ = createNode("array", mallocChildNodes(*(&nodes + 1) - nodes, nodes), $2->value);}};

arrayCommas:                {{$$ = NULL;}}
    | COMMA arrayCommas     {{TreeNode* nodes[] = {$1, $2};$$ = createNode("arrayCommas", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};


/* IF ELSE */

if: IF expr THEN listStatement else END IF {{TreeNode* nodes[] = {$2, $4, $5};$$ = createNode("if", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

else: ELSE listStatement    {{TreeNode* nodes[] = {$2};$$ = createNode("else", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    |                       {{$$ = NULL;}};


while: WHILE expr listStatement WEND {{TreeNode* nodes[] = {$2, $3};$$ = createNode("while", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}

do: DO listStatement LOOP whileOrUntil expr {{TreeNode* nodes[] = {$2, $4, $5};$$ = createNode("do", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}

whileOrUntil: WHILE         {{$$ = $1;}}
    | UNTIL                 {{$$ = $1;}}

break: BREAK {{;$$ = createNode("break", NULL, "");}};

expr: unary                 {{$$ = $1;}}
    | binary                {{$$ = $1;}}
    | braces                {{$$ = $1;}}
    | callOrIndexer         {{$$ = $1;}}
    | place                 {{$$ = $1;}}
    | literal               {{$$ = $1;}};

binary: expr EQUAL expr     {{TreeNode* nodes[] = {$1, $3};$$ = createNode("EQUAL", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};
    | expr PLUS expr        {{TreeNode* nodes[] = {$1, $3};$$ = createNode("PLUS", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr MINUS expr       {{TreeNode* nodes[] = {$1, $3};$$ = createNode("MINUS", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr MUL expr        {{TreeNode* nodes[] = {$1, $3};$$ = createNode("MUL", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr SLASH expr       {{TreeNode* nodes[] = {$1, $3};$$ = createNode("SLASH", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr PERCENT expr     {{TreeNode* nodes[] = {$1, $3};$$ = createNode("PERCENT", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr EQUAL EQUAL expr {{TreeNode* nodes[] = {$1, $4};$$ = createNode("EQUALITY", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr NOTEQUAL expr    {{TreeNode* nodes[] = {$1, $3};$$ = createNode("NOTEQUAL", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr LESSTHAN expr    {{TreeNode* nodes[] = {$1, $3};$$ = createNode("LESSTHAN", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr GREATERTHAN expr {{TreeNode* nodes[] = {$1, $3};$$ = createNode("GREATERTHAN", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr LESSTHANEQ expr  {{TreeNode* nodes[] = {$1, $3};$$ = createNode("LESSTHANEQ", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr GREATERTHANEQ expr   {{TreeNode* nodes[] = {$1, $3};$$ = createNode("GREATERTHANEQ", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr AND expr         {{TreeNode* nodes[] = {$1, $3};$$ = createNode("AND", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | expr OR expr          {{TreeNode* nodes[] = {$1, $3};$$ = createNode("OR", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

unary: PLUS expr            {{TreeNode* nodes[] = {$2};$$ = createNode("PLUS", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | MINUS expr            {{TreeNode* nodes[] = {$2};$$ = createNode("MINUS", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
    | NOT expr              {{TreeNode* nodes[] = {$2};$$ = createNode("NOT", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

braces: LPAREN expr RPAREN  {{TreeNode* nodes[] = {$2};$$ = createNode("braces", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

callOrIndexer: expr LPAREN listExpr RPAREN  {{TreeNode* nodes[] = {$1, $3};$$ = createNode("callOrIndexer", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

listExpr:                   {{$$ = NULL;}}
    | expr COMMA listExpr   {{TreeNode* nodes[] = {$1, $3};$$ = createNode("listExpr", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}

place: IDENTIFIER           {{$$ = $1;}};

literal: TRUE               {{$$ = $1;}}
    | FALSE                 {{$$ = $1;}}
    | STR                   {{$$ = $1;}}
    | CHAR                  {{$$ = $1;}}
    | HEX                   {{$$ = $1;}}
    | BIN                   {{$$ = $1;}}
    | DEC                   {{$$ = $1;}};



/* VAR */

listVar:  {{$$ = NULL;}}
    | IDENTIFIER listVar {{TreeNode* nodes[] = {$1, $2};$$ = createNode("listVar", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}; //чтобы не было listVar с двумя identifier
    | IDENTIFIER COMMA listVar {{TreeNode* nodes[] = {$1, $3};$$ = createNode("listVar", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}};

var: DIM listVar AS typeRef {{TreeNode* nodes[] = {$2, $4};$$ = createNode("var", mallocChildNodes(*(&nodes + 1) - nodes, nodes), "");}}
%%